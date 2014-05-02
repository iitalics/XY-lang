#include "include.h"
#include "expression.h"
#include "lexer.h"
#include "state.h"
#include "value.h"
#include "function.h"
#include "list.h"
#include "syntax.h"
#include "expression.h"

namespace xy {

expression::~expression () { }
bool expression::eval (value& out, state::scope& scope)
{
	scope().error().die() << "Unimplemented expression?";
	return false;
}
bool expression::locate_symbols (const std::shared_ptr<symbol_locator>& locator) { return true; }
bool expression::constant () const { return false; }








bool call_expression::eval (value& out, state::scope& scope)
{
	value func;
	
	if (!func_exp->eval(func, scope))
		return false;
	
	argument_list arg_list(args.size());
	int i = 0;
	for (auto e : args)
		if (!e->eval(arg_list.values[i++], scope))
			return false;
	
	return func.call(out, arg_list, scope());
}

void call_expression::add (const std::shared_ptr<expression>& arg)
{
	args.push_back(arg);
}

bool call_expression::locate_symbols (const std::shared_ptr<symbol_locator>& locator)
{
	if (!func_exp->locate_symbols(locator))
		return false;
	
	for (auto e : args)
		if (!e->locate_symbols(locator))
			return false;
	
	return true;
}






	
	
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
	
private:
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
		
		return va.apply_operator(out, op, vb, scope());
	}
	
	virtual bool constant () const
	{
		return a->constant() && b->constant();
	}
	
	virtual bool locate_symbols (const std::shared_ptr<symbol_locator>& locator)
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
			
		if (!val.apply_unary(out, op, scope()))
			return false;
		
		return true;
	}
	
	virtual bool constant () const
	{
		return a->constant();
	}
	
	virtual bool locate_symbols (const std::shared_ptr<symbol_locator>& locator)
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
	
	virtual bool locate_symbols (const std::shared_ptr<symbol_locator>& locator)
	{
		if (type != unresolved)
			return true;
		
		if (locator->locate(sym, closure_index, closure_depth))
			type = resolved_local;
		else if (locator->env.find_function(sym) != nullptr)
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
