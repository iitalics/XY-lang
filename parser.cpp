#include "include.h"
#include "lexer.h"
#include "state.h"
#include "value.h"
#include "function.h"
#include "list.h"
#include "syntax.h"
#include "expression.h"

namespace xy {





parser::parser (state& p, lexer& l)
	: parent(p), lex(l)
{ }

parser::~parser () {}






std::ostream& symbol_locator::die ()
{
	return parent.error().die_lex(lex);
}

bool symbol_locator::locate (const std::string& sym, int& out_index, int& out_depth)
{
	int index, depth = 0;
	for (auto it = symbols.crbegin(); it != symbols.crend(); it++)
	{
		index = 0;
		
		for (auto s : *it)
			if (s == sym)
			{
				out_index = index;
				out_depth = depth;
				return true;
			}
			else
				index++;
		depth++;
	}
	
	return false;
}
void symbol_locator::push_param_list (const param_list& p)
{
	std::vector<std::string> scope;
	for (int i = 0; i < p.size(); i++)
		scope.push_back(p.param_name(i));
	symbols.push_back(scope);
}
void symbol_locator::pop ()
{
	symbols.pop_back();
}

struct function_generator
{
	std::vector<std::shared_ptr<func_body>> all_bodies;
	
	function_generator () {}
	function_generator (const std::shared_ptr<func_body>& first)
	{
		all_bodies.push_back(first);
	}
	
	bool locate_symbols (const std::shared_ptr<symbol_locator>& locator)
	{
		for (auto body : all_bodies)
		{
			locator->push_param_list(body->params);
			
			for (int i = body->params.size(); i-- > 0; )
			{
				auto cond = body->params.condition(i);
				if (cond != nullptr &&
						!cond->locate_symbols(locator))
					return false;
			}
			
			if (!body->body->locate_symbols(locator))
				return false;
			
			locator->pop();
		}
		return true;
	}
};

bool parser::parse_env (environment& env)
{
	function_generator g;
	
	std::shared_ptr<symbol_locator> old_loc(locator);
	locator = std::shared_ptr<symbol_locator>(new symbol_locator(env, parent, lex));
	
	for (;;)
	{
		if (lex.current().tok == lexer::token::keyword_let)
		{
			if (!parse_declare(env, g))
				return false;
		}
		// else if ( OTHER TOKEN )
		else
			break;
	}
	
	bool r = g.locate_symbols(locator);
	locator = old_loc;
	return r;
}

bool parser::parse_declare (environment& env, function_generator& g)
{
	if (!lex.expect(lexer::token::keyword_let, true))
		return false;
	
	std::shared_ptr<soft_function> soft_func;
	
	if (!lex.expect(lexer::token::symbol_token, false))
		return false;
		
	std::string func_name(lex.current().str);
	
	if (!lex.advance())
		return false;
	
	// find function
	std::shared_ptr<function> hard_func =
		env.find_function(func_name);
	
	if (hard_func == nullptr)
	{
		// create new function
		soft_func = std::shared_ptr<soft_function>(
						new soft_function(func_name));
		env.add_function(soft_func);
	}
	else if (hard_func->is_native())
	{
		parent.error().die_lex(lex)
			<< "Cannot overload native function '" << func_name << "'";
		return false;
	}
	else
		soft_func = std::static_pointer_cast<soft_function>(hard_func); // UGLY :/
	
	
	std::shared_ptr<func_body> body;
	if (!parse_function(body))
		return false;
		
	soft_func->add_overload(body);
	g.all_bodies.push_back(body);
	
	return true;
}




bool parser::parse_function (std::shared_ptr<func_body>& out)
{
	if (!lex.expect(SYNTAX_FUNC_L, true))
		return false;
	
	std::shared_ptr<func_body> body(new func_body());
	
	while (lex.current().tok != SYNTAX_FUNC_R)
	{
		if (lex.current().tok == lexer::token::symbol_token)
		{
			// TODO: refactor me
			if (!lex.expect(lexer::token::symbol_token, false))
				return false;
			
			std::string param_name(lex.current().str);
			
			if (!lex.advance())
				return false;
			
			if (body->params.locate(param_name) != -1)
			{
				parent.error().die_lex(lex)
					<< "Parameter '" << param_name << "' already declared";
				return false;
			}
			
			if (lex.current().tok == SYNTAX_FUNC_PCOND)
			{
				if (!lex.advance())
					return false;
				
				std::shared_ptr<expression> cond;
				if (!parse_exp(cond))
					return false;
				
				body->params.add_param(param_name, cond);
			}
			else if (lex.current().is_binary_op())
			{
				int op = lex.current().tok;
				if (!lex.advance())
					return false;
				std::shared_ptr<expression> right;
				if (!parse_single_exp(right))
					return false;
				
				body->params.add_param(param_name,
					expression::create_binary(expression::create_symbol(param_name),
					                          right,
											  op));
			}
			else
				body->params.add_param(param_name);
		}
		else if (lex.current().is_expression())
		{
			std::shared_ptr<expression> exp;
			if (!parse_exp(exp))
				return false;
			body->params.add_param(exp);
		}
		else
			return lex.unexpect();
		
		
		if (lex.current().tok == SYNTAX_FUNC_PSEP)
		{
			if (!lex.advance())
				return false;
			continue;
		}
		else if (lex.current().tok == SYNTAX_FUNC_R)
		{
			break;
		}
		else
		{
			parent.error().die_lex(lex)
				<< "Expected '" << SYNTAX_FUNC_PSEP << "' or '" << SYNTAX_FUNC_R
				<< "', got '" << lex.current().to_str() << "'";
			return false;
		}
	}
	
	if (!lex.advance() ||
			!lex.expect(SYNTAX_FUNC_PROLOGUE, true))
		return false;
	if (!parse_exp(body->body))
		return false;
	
	out = body;
	return true;
}










bool parser::parse_single_exp (std::shared_ptr<expression>& out)
{
	switch (lex.current().tok)
	{
	case lexer::token::number_token:
		out = expression::create_const(value::from_number(lex.current().num));
		break;
		
	case lexer::token::symbol_token:
		out = expression::create_symbol(lex.current().str);
		break;
		
	case lexer::token::keyword_true:
		out = expression::create_const(value::from_bool(true));
		break;
		
	case lexer::token::keyword_false:
		out = expression::create_const(value::from_bool(false));
		break;
		
	case SYNTAX_LAMBDA:
		if (!parse_lambda(out))
			return false;
		goto prologue;
		
	case SYNTAX_LPAREN:
		if (!lex.advance())
			return false;
		
		if (lex.current().tok == SYNTAX_RPAREN)
		{
			out = expression::create_const(value());
			break;
		}
		
		if (!parse_exp(out))
			return false;
		if (!lex.expect(SYNTAX_RPAREN))
			return false;
		break;
	
	case SYNTAX_LIST_L:
		if (!parse_list(out))
			return false;
		goto prologue;
	
	default:
		if (lex.current().is_unary_op())
		{
			int op = lex.current().tok;
			
			std::shared_ptr<expression> a;
			if (!lex.advance())
				return false;
			if (!parse_single_exp(a))
				return false;
			
			out = expression::create_unary(a, op);
			goto prologue;
		}
		return lex.unexpect();
	}
	
	if (!lex.advance())
		return false;
	
prologue:
	return parse_exp_prologe(out, out);// yikes
}

struct shunting_yard
{
	shunting_yard (const std::shared_ptr<expression>& first)
	{
		exp_stack.push_back(first);
	}
	
	void apply_top ()
	{
		int op;
		std::shared_ptr<expression> a, b;
		
		b = exp_stack.back();
		exp_stack.pop_back();
		a = exp_stack.back();
		exp_stack.pop_back();
		
		op = op_stack.back();
		op_stack.pop_back();
		
		exp_stack.push_back(expression::create_binary(a, b, op));
	}
	
	void push_exp (const std::shared_ptr<expression>& e)
	{
		exp_stack.push_back(e);
	}
	
	void push_op (int op)
	{
		int p = precedence(op);
		
		while (op_stack.size() > 0 &&
				precedence(op_stack.back()) >= p)
			apply_top();
		
		op_stack.push_back(op);
		
	}
	
	
	static int precedence (int op)
	{
		switch (op)
		{
		case '^':
			return 6;
		
		case '.': case lexer::token::seq_token:
			return 5;
		
		case '*': case '/':
			return 4;
		
		case '+': case '-':
			return 3;
		
		case lexer::token::eql_token:
		case lexer::token::neq_token:
		case lexer::token::gre_token:
		case lexer::token::lse_token:
		case '<': case '>':
			return 2;
		
		case lexer::token::keyword_or:
		case lexer::token::keyword_and:
			return 1;
		
		default:
			return 0;
		}
	}
	
	
	std::vector<std::shared_ptr<expression>>
		exp_stack;
	std::vector<int> op_stack;
	
	std::shared_ptr<expression> finish ()
	{
		while (op_stack.size() > 0)
			apply_top();
		
		return exp_stack.back();
	}
};

bool parser::parse_exp (std::shared_ptr<expression>& out)
{
	std::shared_ptr<expression> exp;
	
	if (!parse_single_exp(exp))
		return false;
	
	shunting_yard builder(exp);
	
	while (lex.current().is_binary_op())
	{
		builder.push_op(lex.current().tok);
		if (!lex.advance())
			return false;
		if (!parse_single_exp(exp))
			return false;
		builder.push_exp(exp);
	}
	out = builder.finish();
	return true;
}









bool parser::parse_exp_prologe (std::shared_ptr<expression>& out, std::shared_ptr<expression> in)
{
	for (;;)
	{
		if (lex.current().tok == SYNTAX_FUNC_L)
		{
			if (!lex.advance())
				return false;
			
			std::shared_ptr<call_expression> ce(new call_expression(in));
			while (lex.current().tok != SYNTAX_FUNC_R)
			{
				std::shared_ptr<expression> arg;
				if (!parse_exp(arg))
					return false;
				ce->add(arg);
				
				if (lex.current().tok == SYNTAX_FUNC_PSEP)
				{
					if (!lex.advance())
						return false;
				}
				else if (lex.current().tok != SYNTAX_FUNC_R)
				{
					parent.error().die_lex(lex)
						<< "Expected '" << SYNTAX_FUNC_PSEP << "' or '" << SYNTAX_FUNC_R
						<< "', got '" << lex.current().to_str() << "'";
					return false;
				}
			}
			
			if (!lex.advance())
				return false;
			
			in = ce;
		}
		// if (lex.current().tok == lex::token::rarrow_token)
		else
			break;
	}
	out = in;
	return true;
}















class list_expression : public expression
{
public:
	list_expression () {}
	
	
	virtual bool eval (value& out, state::scope& scope)
	{
		if (items.size() == 0)
		{
			out = value::from_list(list::empty());
			return true;
		}
		std::vector<value> vs;
		value v;
		
		for (auto e : items)
			if (!e->eval(v, scope))
				return false;
			else
				vs.push_back(v);
		
		out = value::from_list(std::shared_ptr<list>(new list_basic(vs)));
		return true;
	}
	virtual bool locate_symbols (const std::shared_ptr<symbol_locator>& locator)
	{
		for (auto e : items)
			if (!e->locate_symbols(locator))
				return false;
		return true;
	}
	virtual bool constant () const
	{
		for (auto e : items)
			if (!e->constant())
				return false;
		return true;
	}
	
	inline void add (const std::shared_ptr<expression>& arg)
	{
		items.push_back(arg);
	}
	
private:
	std::vector<std::shared_ptr<expression>> items;
};


bool parser::parse_list (std::shared_ptr<expression>& out)
{
	if (!lex.expect(SYNTAX_LIST_L, true))
		return false;
	
	std::shared_ptr<list_expression> le(new list_expression());
	std::shared_ptr<expression> e;
	
	while (lex.current().tok != SYNTAX_LIST_R)
	{
		if (!parse_exp(e))
			return false;
		le->add(e);
		
		if (lex.current().tok == SYNTAX_LIST_SEP)
		{
			if (!lex.advance())
				return false;
		}
		else if (lex.current().tok != SYNTAX_LIST_R)
		{
			parent.error().die_lex(lex)
				<< "Expected '" << SYNTAX_LIST_SEP << "' or '" << SYNTAX_LIST_R
				<< "', got '" << lex.current().to_str() << "'";
			return false;
		}
	}
	if (!lex.advance())
		return false;
	
	out = le;
	return true;
}



class lambda_expression
	: public expression
{
public:
	lambda_expression ()
	{ }
	
	
	virtual bool eval (value& out, state::scope& scope)
	{
		std::shared_ptr<soft_function> func(new soft_function(scope.local));
		for (auto body : g.all_bodies)
			func->add_overload(body);
		out = value::from_function(func);
		return true;
	}
	
	virtual bool locate_symbols (const std::shared_ptr<symbol_locator>& locator)
	{
		return g.locate_symbols(locator);
	}
	
	void add (const std::shared_ptr<func_body>& body)
	{
		g.all_bodies.push_back(body);
	}
private:
	function_generator g;
};

bool parser::parse_lambda (std::shared_ptr<expression>& out)
{
	std::shared_ptr<lambda_expression> e(new lambda_expression());
	std::shared_ptr<func_body> body;
	
	if (!lex.expect(SYNTAX_LAMBDA, true))
		return false;
	
	if (lex.current().tok == SYNTAX_FUNC_L)
	{
		if (!parse_function(body))
			return false;
		e->add(body);
	}
	else
	{
		if (!lex.expect(SYNTAX_LAMBDA_L, true))
			return false;
		
		while (lex.current().tok != SYNTAX_LAMBDA_R)
		{
			if (!lex.expect(lexer::token::keyword_let, true))
				return false;
			
			if (!parse_function(body))
				return false;
			e->add(body);
		}
		
		if (!lex.advance())
			return false;
	}
	
	out = e;
	return true;
}








};
