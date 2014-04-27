#include "include.h"
#include "parser.h"
#include "lexer.h"
#include "state.h"
#include "value.h"
#include "function.h"

namespace xy {


parser::parser (state& p, lexer& l)
	: parent(p), lex(l)
{
}

parser::~parser ()
{
}


struct function_generator
{
	function_generator ()
		: last(nullptr)
	{
	}
	
	soft_function* last;
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
	
	return true;
}

bool parser::parse_declare (environment& env, function_generator& g)
{
	if (!lex.expect(lexer::token::keyword_let, true))
		return false;
	if (!lex.expect(lexer::token::symbol_token, false))
		return false;
		
	std::string func_name(lex.current().str);
	
	// find function!!
	std::shared_ptr<function> hard_func =
		env.find_function(func_name);
	std::shared_ptr<soft_function> soft_func;
	
	if (hard_func == nullptr)
	{
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
		soft_func = std::dynamic_pointer_cast<soft_function>(hard_func); // UGLY :/
	
	if (!lex.advance() ||
			!lex.expect('(', true))
		return false;
	
	std::shared_ptr<soft_function::func_body> body(
		new soft_function::func_body());
	
	while (lex.current().tok != ')')
	{
		// TODO: conditions
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
		body->params.add_param(param_name);
		
		
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
	
	soft_func->add_overload(body);
	
	return true;
}







bool parser::parse_single_exp (std::shared_ptr<expression>& out)
{
	switch (lex.current().tok)
	{
	case lexer::token::number_token:
		out = expression::create_const(value::from_number(lex.current().num));
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
		return lex.unexpect();
	}
	
	return lex.advance();
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









////  expression structure

expression::~expression () { }
bool expression::eval (value& out, state::scope& scope)
{
	scope().error().die() << "Unimplemented expression?";
	return false;
}
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
		
		if (!va.apply_operator(out, op, vb))
		{
			scope().error().die()
				<< "Cannot apply operator '" << lexer::token(op).to_str()
				<< "' to values of type '" << va.type_string() << "' and '" << vb.type_string() << "'";
			return false;
		}
		
		return true;
	}
	
	virtual bool constant () const
	{
		return a->constant() && b->constant();
	}
	
private:
	std::shared_ptr<expression> a, b;
	int op;
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
std::shared_ptr<expression> expression::create_true ()
{
	return std::shared_ptr<expression>(new const_exp(value::from_bool(true)));
}


};
