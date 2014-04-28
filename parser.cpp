#include "include.h"
#include "parser.h"
#include "lexer.h"
#include "state.h"
#include "value.h"
#include "function.h"

namespace xy {


parser::parser (state& p, lexer& l)
	: parent(p), lex(l)
{ }

parser::~parser () {}



struct function_generator
{
	std::vector<std::shared_ptr<func_body>> all_bodies;
};

struct symbol_locator
{
	symbol_locator (environment& e, state& s, lexer& l)
		: env(e), parent(s), lex(l)
	{ }
	
	environment& env;
	state& parent;
	lexer& lex;
	
	std::vector<std::vector<std::string>> symbols;
	
	
	std::ostream& die ()
	{
		return parent.error().die_lex(lex);
	}
	
	bool locate (const std::string& sym, int& out_index, int& out_depth)
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
	}/*
	bool locate_global (std::shared_ptr<function>& out, const std::string& sym)
	{
		auto f = env.find_function(sym);
		if (f == nullptr)
			return false;
		
		out = f;
		return true;
	}*/
	
	void push_param_list (const param_list& p)
	{
		std::vector<std::string> scope;
		for (int i = 0; i < p.size(); i++)
			scope.push_back(p.param_name(i));
		symbols.push_back(scope);
	}
	void pop ()
	{
		symbols.pop_back();
	}
};

bool parser::parse_env (environment& env)
{
	function_generator g;
	
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
	
	symbol_locator locator(env, parent, lex);
	
	for (auto body : g.all_bodies)
	{
		locator.push_param_list(body->params);
		
		for (int i = body->params.size(); i-- > 0; )
		{
			auto cond = body->params.condition(i);
			if (!cond->locate_symbols(locator))
				return false;
		}
		
		if (!body->body->locate_symbols(locator))
			return false;
		
		locator.pop();
	}
	
	return true;
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
	if (!lex.expect('(', true))
		return false;
	
	std::shared_ptr<func_body> body(new func_body());
	
	while (lex.current().tok != ')')
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
			
			if (lex.current().tok == ':')
			{
				if (!lex.advance())
					return false;
				
				std::shared_ptr<expression> cond;
				if (!parse_exp(cond))
					return false;
				
				body->params.add_param(param_name, cond);
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
		
		
		if (lex.current().tok == ',')
		{
			if (!lex.advance())
				return false;
			continue;
		}
		else if (lex.current().tok == ')')
		{
			break;
		}
		else
		{
			parent.error().die_lex(lex)
				<< "Expected ',' or ')', got '" << lex.current().to_str() << "'";
			return false;
		}
	}
	
	if (!lex.advance() ||
			!lex.expect('=', true))
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
		
	case '(':
		if (!lex.advance())
			return false;
		
		if (lex.current().tok == ')') // void
		{
			out = expression::create_const(value());
			break;
		}
		
		if (!parse_exp(out))
			return false;
		if (!lex.expect(')'))
			return false;
		break;
	
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
			return true;
		}
		return lex.unexpect();
	}
	
	if (!lex.advance())
		return false;
	
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
	
	static int precedence (int op)
	{
		switch (op)
		{
		case '^':
			return 5;
		
		case '*': case '/':
			return 4;
		
		case '+': case '-':
			return 3;
		
		case lexer::token::eql_token:
		case lexer::token::neq_token:
		case lexer::token::gre_token:
		case lexer::token::lse_token:
			return 2;
		
		case lexer::token::keyword_or:
		case lexer::token::keyword_and:
			return 1;
		
		default:
			return 0;
		}
	}
	
	void push_op (int op)
	{
		int p = precedence(op);
		
		while (op_stack.size() > 0 &&
				precedence(op_stack.back()) >= p)
			apply_top();
		
		op_stack.push_back(op);
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








class call_expression : public expression
{
public:
	call_expression (const std::shared_ptr<expression>& func)
		: func_exp(func)
	{ }
	
	virtual bool eval (value& out, state::scope& scope)
	{
		value func;
		
		if (!func_exp->eval(func, scope))
			return false;
		
		argument_list arg_list(args.size());
		int i = 0;
		for (auto e : args)
			if (!e->eval(arg_list.values[i++], scope))
				return false;
		
		return func.call(out, arg_list, scope);
	}
	
	inline void add (const std::shared_ptr<expression>& arg)
	{
		args.push_back(arg);
	}
	
	virtual bool locate_symbols (symbol_locator& locator)
	{
		if (!func_exp->locate_symbols(locator))
			return false;
		
		for (auto e : args)
			if (!e->locate_symbols(locator))
				return false;
		
		return true;
	}
private:
	std::shared_ptr<expression> func_exp;
	std::vector<std::shared_ptr<expression>> args;
};


bool parser::parse_exp_prologe (std::shared_ptr<expression>& out, std::shared_ptr<expression> in)
{
	for (;;)
	{
		if (lex.current().tok == '(')
		{
			if (!lex.advance())
				return false;
			
			std::shared_ptr<call_expression> ce(new call_expression(in));
			while (lex.current().tok != ')')
			{
				std::shared_ptr<expression> arg;
				if (!parse_exp(arg))
					return false;
				ce->add(arg);
				
				if (lex.current().tok == ',')
				{
					if (!lex.advance())
						return false;
				}
				else if (lex.current().tok != ')')
				{
					parent.error().die_lex(lex)
						<< "Expected ',' or ')', got '" << lex.current().to_str() << "'";
					return false;
				}
			}
			
			if (!lex.advance())
				return false;
			
			in = ce;
		}
		break;
	}
	out = in;
	return true;
}









////  expression structure

expression::~expression () { }
bool expression::eval (value& out, state::scope& scope)
{
	scope().error().die() << "Unimplemented expression?";
	return false;
}
bool expression::locate_symbols (symbol_locator& locator) { return true; }
bool expression::constant () const { return false; }



	
	
class const_exp : public expression
{
public:
	inline const_exp (const value& v)
		: val(v)
	{}
	
	virtual bool eval (value& out, state::scope& scope)
	{
		out = val;
		return true;
	}
	virtual bool constant () const { return true; }
	
	value val;
};
class binary_exp : public expression
{
public:
	binary_exp (const std::shared_ptr<expression>& ea,
					const std::shared_ptr<expression>& eb,
					int opr)
		: a(ea), b(eb), op(opr)
	{}
	
	virtual bool eval (value& out, state::scope& scope)
	{
		value va, vb;
		
		if (!a->eval(va, scope))
			return false;
		
		if (op == lexer::token::keyword_and ||
				op == lexer::token::keyword_or)
		{
			// a & b  ==   a ? b : a    ==  !a ? a : b
			// a | b  ==   a ? a : b    ==   a ? a : bfuckkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk
			
			bool cond = va.condition();
			if (op == lexer::token::keyword_and)
				cond = !cond;
			
			if (cond)
			{
				out = va;
				return true;
			}
			else
				return b->eval(out, scope);
		}
		
		if (!b->eval(vb, scope))
			return false;
		
		return va.apply_operator(out, op, vb, scope().error());
	}
	
	virtual bool constant () const
	{
		return a->constant() && b->constant();
	}
	
	virtual bool locate_symbols (symbol_locator& locator)
	{
		return a->locate_symbols(locator) &&
			b->locate_symbols(locator);
	}
	
private:
	std::shared_ptr<expression> a, b;
	int op;
};

class unary_exp : public expression
{
public:
	unary_exp (const std::shared_ptr<expression>& e,
					int opr)
		: a(e), op(opr)
	{ }
	
	virtual bool eval (value& out, state::scope& scope)
	{
		value val;
		if (!a->eval(val, scope))
			return false;
			
		if (!val.apply_unary(out, op, scope().error()))
			return false;
		
		return true;
	}
	
	virtual bool constant () const
	{
		return a->constant();
	}
	
	virtual bool locate_symbols (symbol_locator& locator)
	{
		return a->locate_symbols(locator);
	}
	
private:
	std::shared_ptr<expression> a;
	int op;
};


class symbol_exp : public expression
{
public:
	symbol_exp (const std::string& s)
		: type( unresolved), sym(s)
	{ }
	
	symbol_exp (int index, int depth)
		: type(resolved_local), closure_index(index), closure_depth(depth)
	{ }
	
	virtual bool eval (value& out, state::scope& scope)
	{
		switch (type)
		{
		case resolved_local:
			out = scope.local->get(closure_index, closure_depth);
			return true;
			
		case resolved_global:
			{
				auto func = scope().global().find_function(sym);
				if (func == nullptr) // this shouldn't happen ever
					out.type = value::type_void;
				else
					out = value::from_function(func);
				return true;
			}
			
		default:
			scope().error().die()
				<< "Use of unresolved symbol '" << sym << "'";
			return false;
		}
	}
	
	virtual bool locate_symbols (symbol_locator& locator)
	{
		if (type != unresolved)
			return true;
		
		if (locator.locate(sym, closure_index, closure_depth))
			type = resolved_local;
		else if (locator.env.find_function(sym) != nullptr)
			type = resolved_global;
		
		return true;
	}
	
private:
	enum resolve_type
	{
		unresolved,
		resolved_global,
		resolved_local
	};
	resolve_type type;
	std::string sym;
	int closure_index, closure_depth;
};

state expression::constant_state;

std::shared_ptr<expression> expression::create_const (const value& val)
{
	return std::shared_ptr<expression>(new const_exp(val));
}
std::shared_ptr<expression> expression::create_binary (const std::shared_ptr<expression>& a,
											const std::shared_ptr<expression>& b,
											int op)
{
	return std::shared_ptr<expression>(new binary_exp(a, b, op));
}
std::shared_ptr<expression> expression::create_symbol (const std::string& sym)
{
	return std::shared_ptr<expression>(new symbol_exp(sym));
}
std::shared_ptr<expression> expression::create_closure_ref (int index, int depth)
{
	return std::shared_ptr<expression>(new symbol_exp(index, depth));
}
std::shared_ptr<expression> expression::create_true ()
{
	return std::shared_ptr<expression>(new const_exp(value::from_bool(true)));
}
std::shared_ptr<expression> expression::create_unary (const std::shared_ptr<expression>& a, int op)
{
	return std::shared_ptr<expression>(new unary_exp(a, op));
}

};
