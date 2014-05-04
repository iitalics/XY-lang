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

expression::~expression () {}
bool expression::eval (value& out, state::scope& scope)
{
	scope().error().die() << "Unimplemented expression?";
	return false;
}
bool expression::eval_tail_call (tail_call& tc, value& out, state::scope& scope)
{
	tc.do_tail = false;
	return eval(out, scope);
}
bool expression::locate_symbols (const std::shared_ptr<symbol_locator>& locator) { return true; }
bool expression::constant () const { return false; }


expression::tail_call::tail_call (function* f)
	: func(f), do_tail(false)
{ }



std::ostream& symbol_locator::die ()
{
	return parent.error().die_lex(lex);
}

bool symbol_locator::locate (const std::string& sym, int& out_index, int& out_depth)
{
	int index, depth = 0;
	
	// search backwards
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
void symbol_locator::push_empty ()
{
	symbols.push_back(std::vector<std::string>());
}
void symbol_locator::push_param_list (const param_list& p)
{
	push_empty();
	for (int i = 0; i < p.size(); i++)
		add(p.param_name(i));
}
void symbol_locator::add (const std::string& name)
{
	symbols.back().push_back(name);
}
void symbol_locator::pop ()
{
	symbols.pop_back();
}








bool call_expression::eval_tail_call (tail_call& tc, value& out, state::scope& scope)
{
	value func;
	
	if (!func_exp->eval(func, scope))
		return false;
	
	argument_list arg_list(args.size());
	int i = 0;
	for (auto e : args)
		if (!e->eval(arg_list.values[i++], scope))
			return false;
	
	if (tc.func != nullptr &&
			func.type == value::type_function &&
			func.func_obj.get() == tc.func)
	{
		tc.do_tail = true;
		for (i = 0; i < arg_list.size; i++)
			tc.args.push_back(arg_list.values[i]);
		
		return true;
	}
	else
		tc.do_tail = false;
	
	return func.call(out, arg_list, scope());
}
bool call_expression::eval (value& out, state::scope& scope)
{
	tail_call tc(nullptr);
	return eval_tail_call(tc, out, scope);
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



	
list_expression::list_expression () {}
bool list_expression::eval (value& out, state::scope& scope)
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
bool list_expression::locate_symbols (const std::shared_ptr<symbol_locator>& locator)
{
	for (auto e : items)
		if (!e->locate_symbols(locator))
			return false;
	return true;
}
bool list_expression::constant () const
{
	for (auto e : items)
		if (!e->constant())
			return false;
	return true;
}
void list_expression::add (const std::shared_ptr<expression>& arg)
{
	items.push_back(arg);
}




list_comp_expression::list_comp_expression (const std::shared_ptr<expression>& origin, const std::string& n)
	: it_name(n), start(origin), filter(nullptr), map(nullptr)
{ }
bool list_comp_expression::eval (value& out, state::scope& scope)
{
	value list_val, item, filt_result;
	
	if (!start->eval(list_val, scope))
		return false;
	
	if (list_val.type != value::type_list)
	{
		scope().error().die()
			<< "Cannot process list comprehension on value of type '"
			<< list_val.type_str() << "'";
		return false;
	}
	
	state::scope new_scope(scope.parent, // re-use this scope
		std::shared_ptr<closure>(new closure(1, scope.local)));
	
	std::vector<value> output;
	int size = list_val.list_obj->size();
	for (int i = 0; i < size; i++)
	{
		item = list_val.list_obj->get(i);
		new_scope.local->set(0, item); // set one argument, being the iterator
		
		if (filter != nullptr)
		{
			if (!filter->eval(filt_result, new_scope))
				return false;
			
			if (!filt_result.condition())
				continue; // do not insert
		}
		if (map != nullptr)
			if (!map->eval(item, new_scope))
				return false;
		
		output.push_back(item);
	}
	if (output.size() == 0)
		out = value::from_list(list::empty());
	else
		out = value::from_list(std::shared_ptr<list>(new list_basic(output)));
	return true;
}
bool list_comp_expression::locate_symbols (const std::shared_ptr<symbol_locator>& locator)
{
	if (!start->locate_symbols(locator))
		return false;
	
	locator->push_empty();
	locator->add(it_name);
	
	if (filter != nullptr && !filter->locate_symbols(locator))
			return false;
	if (map != nullptr && !map->locate_symbols(locator))
			return false;
	
	locator->pop();
	return true;
}
bool list_comp_expression::constant () const { return false; }











bool with_expression::eval_tail_call (tail_call& tc, value& out, state::scope& parent_scope)
{
	state::scope scope(parent_scope(),
		std::shared_ptr<closure>(new closure(vars.size(), parent_scope.local)));
	value item;
	
	int i = 0;
	for (auto& v : vars)
	{
		if (!v.val->eval(item, scope))
			return false;
		
		scope.local->set(i++, item);
	}
	
	return body->eval_tail_call(tc, out, scope);
}
bool with_expression::eval (value& out, state::scope& scope)
{
	tail_call tc(nullptr);
	return eval_tail_call(tc, out, scope);
}
bool with_expression::locate_symbols (const std::shared_ptr<symbol_locator>& locator)
{
	locator->push_empty();
	for (auto& v : vars)
	{
		if (!v.val->locate_symbols(locator))
			return false;
		
		locator->add(v.name);
	}
	
	if (!body->locate_symbols(locator))
		return false;
	
	locator->pop();
	return true;
}
bool with_expression::constant () const { return false; }

bool with_expression::add (const std::string& name, const std::shared_ptr<expression>& val)
{
	for (auto& v : vars)
		if (v.name == name)
			return false;
	
	vars.push_back({ name, val });
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
std::shared_ptr<expression> expression::create_unary (const std::shared_ptr<expression>& a, int op)
{
	return std::shared_ptr<expression>(new unary_exp(a, op));
}






};
