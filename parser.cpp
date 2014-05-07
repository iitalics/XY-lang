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






struct function_generator
{
	std::vector<std::shared_ptr<func_body>> all_bodies;
	std::string last_function;
	
	function_generator ()
		: last_function("")
	{ }
	
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

class lambda_expression
	: public expression
{
public:
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
		else if (lex.current().tok == lexer::token::keyword_use)
		{
			if (!lex.advance())
				return false;
			if (!lex.expect(lexer::token::string_token))
				return false;
			std::string fname(lex.current().str);
			if (!lex.advance())
				return false;
			
			lexer sublex(parent);
			if (!sublex.open(fname))
				return false;
			parser subparser(parent, sublex);
			if (!subparser.parse_env(env))
				return false;
		}
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
	
	std::string func_name;
	
	if (lex.current().tok == lexer::token::seq_token)
	{
		if (g.last_function == "")
		{
			parent.error().die_lex(lex)
				<< "Cannot recall previous function: first function in environment";
			return false;
		}
		func_name = g.last_function;
	}
	else
	{
		if (!lex.expect(lexer::token::symbol_token, false))
			return false;
		func_name = lex.current().str;
		
		g.last_function = func_name;
	}
	
	if (!lex.advance())
		return false;
	
	std::shared_ptr<soft_function> soft_func =
		env.find_or_add(func_name);
	
	if (soft_func == nullptr)
	{
		parent.error().die_lex(lex)
			<< "Cannot overload function '" << func_name << "'";
		return false;
	}
	
	
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
	
	if (lex.current().tok != SYNTAX_FUNC_R)
	for (;;)
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
	int first;
	switch (first = lex.current().tok)
	{
	case lexer::token::number_token:
		out = expression::create_const(value::from_number(lex.current().num));
		break;
		
	case lexer::token::string_token:
		out = expression::create_const(value::from_string(lex.current().str));
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
	
	case lexer::token::keyword_with:
		if (!parse_with(out))
			return false;
		goto prologue;
	
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
	
	case SYNTAX_MINI_LAMBDA_BIN:
	case SYNTAX_MINI_LAMBDA_LEFT:
		if (!lex.advance())
			return false;
		//std::cout << "binary mini-lambda: " << lex.current().to_str() << std::endl;
		if (!lex.current().is_binary_op())
		{
			parent.error().die_lex(lex)
				<< "Expected binary operator, got '" << lex.current().to_str() << "'";
			return false;
		}
		{
			std::shared_ptr<expression> body, right, left;
			int op = lex.current().tok;
			if (!lex.advance())
				return false;
			
			if (first == SYNTAX_MINI_LAMBDA_BIN)
			{
				left = expression::create_closure_ref(0);
				right = expression::create_closure_ref(1);
			}
			else // if (first == SYNTAX_MINI_LAMBDA_LEFT)
			{
				left = expression::create_closure_ref(0);
				if (!parse_exp(right))
					return false;
			}
			body = expression::create_binary(left, right, op);
			
			std::shared_ptr<func_body> fb(new func_body());
			fb->body = body;
			
			std::shared_ptr<lambda_expression> le(new lambda_expression());
			le->add(fb);
			
			out = le;
			goto prologue;
		}
		
	
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
			return 7;
		
		case '.': case lexer::token::seq_token:
			return 6;
		
		case '*': case '/':
			return 5;
		
		case '+': case '-':
			return 4;
		
		case lexer::token::rarr_token:
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
	
	if (lex.current().tok == SYNTAX_LCOMP_SEP)
		if (!parse_list_comp(out, out))
			return false;
	
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
		else
			break;
	}
	out = in;
	return true;
}















bool parser::parse_list (std::shared_ptr<expression>& out)
{
	if (!lex.expect(SYNTAX_LIST_L, true))
		return false;
	
	std::shared_ptr<list_expression> le(new list_expression());
	std::shared_ptr<expression> e;
	
	if (lex.current().tok != SYNTAX_LIST_R)
		for (;;)
		{
			if (!parse_exp(e))
				return false;
			le->add(e);
			
			if (lex.current().tok == SYNTAX_LIST_SEP)
			{
				if (!lex.advance())
					return false;
			}
			else if (lex.current().tok == SYNTAX_LIST_R)
			{
				break;
			}
			else
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



bool parser::parse_list_comp (std::shared_ptr<expression>& out, const std::shared_ptr<expression>& list_exp)
{
	if (!lex.expect(SYNTAX_LCOMP_SEP, true))
		return false;
	if (!lex.expect(lexer::token::symbol_token))
		return false;
	
	std::shared_ptr<expression> e;
	std::shared_ptr<list_comp_expression> comp(new list_comp_expression(list_exp, lex.current().str));
	if (!lex.advance())
		return false;
	
	bool any = false;
	
	if (lex.current().tok == SYNTAX_LCOMP_FILTER)
	{
		if (!lex.advance())
			return false;
		if (!parse_exp(e))
			return false;
		comp->set_filter(e);
		any = true;
	}
	
	if (lex.current().tok == SYNTAX_LCOMP_MAP)
	{
		if (!lex.advance())
			return false;
		if (!parse_exp(e))
			return false;
		comp->set_map(e);
		any = true;
	}
	
	if (!any)
	{
		parent.error().die_lex(lex)
			<< "Expected '" << SYNTAX_LCOMP_FILTER
			<< "' or '"<< SYNTAX_LCOMP_MAP << "' case in list comprehension";
		return false;
	}
	
	out = comp;
	return true;
}



bool parser::parse_with (std::shared_ptr<expression>& out)
{
	if (!lex.expect(lexer::token::keyword_with, true))
		return false;
	if (!lex.expect(SYNTAX_WITH_L, true))
		return false;
	
	std::shared_ptr<with_expression> w(new with_expression());
	std::shared_ptr<expression> e;
	
	if (lex.current().tok != SYNTAX_WITH_R)
	for (;;)
	{
		if (lex.current().tok == lexer::token::symbol_token)
		{
			std::string name(lex.current().str);
			if (!lex.advance())
				return false;
			if (!lex.expect(SYNTAX_WITH_ASSIGN, true))
				return false;
			if (!parse_exp(e))
				return false;
				
			if (!w->add(name, e))
			{
				parent.error().die_lex(lex)
					<< "Duplicate declaration of alias '" << name << "'";
				return false;
			}
		}
		else if (lex.current().tok == SYNTAX_LIST_L)
		{
			if (!lex.advance())
				return false;
			
			std::vector<std::string> names;
			bool var = false;
			
			while (lex.current().tok != SYNTAX_LIST_R)
			{
				if (!lex.expect(lexer::token::symbol_token))
					return false;
				names.push_back(lex.current().str);
				if (!lex.advance())
					return false;
				
				if (lex.current().tok == SYNTAX_LIST_SEP)
				{
					if (!lex.advance())
						return false;
					continue;
				}
				if (lex.current().tok == lexer::token::seq_token)
				{
					if (!lex.advance())
						return false;
					var = true;
				}
				if (lex.current().tok != SYNTAX_LIST_R)
				{
					parent.error().die_lex(lex)
						<< "Expected '" << SYNTAX_LIST_SEP << "' or '" << SYNTAX_WITH_R
						<< "', got '" << lex.current().to_str() << "'";
					return false;
				}
			}
			if (!lex.advance())
				return false;
			if (!lex.expect(SYNTAX_WITH_ASSIGN, true))
				return false;
			if (!parse_exp(e))
				return false;
				
			if (!w->add_list(names, e, var))
			{
				parent.error().die_lex(lex)
					<< "Duplicate declaration of alias in list";
				return false;
			}
		}
		
		if (lex.current().tok == SYNTAX_WITH_SEP)
		{
			if (!lex.advance())
				return false;
		}
		else if (lex.current().tok == SYNTAX_WITH_R)
		{
			break;
		}
		else
		{
			parent.error().die_lex(lex)
				<< "Expected '" << SYNTAX_WITH_SEP << "' or '" << SYNTAX_WITH_R
				<< "', got '" << lex.current().to_str() << "'";
			return false;
		}
	}
	
	if (w->empty())
	{
		parent.error().die_lex(lex)
			<< "Invalid 'with' expression without any declared aliases";
		return false;
	}
	
	if (!lex.advance())
		return false;
	if (!parse_exp(e))
		return false;
	
	w->set_body(e);
	out = w;
	return true;
}


};
