#include "include.h"
#include "function.h"
#include "value.h"
#include "lexer.h"
#include "parser.h"
#include "expression.h"

namespace xy {


// not sure about this one, yet
/*  #define XY_REVERSE_OVERLOAD_ORDER */


function::function (const std::string& name, bool n)
	: func_name(name), native(n)
{ }

function::~function () {}

bool function::call (value& out, const argument_list& args, state::scope& scope)
{
	out.type = value::type_void;
	return true;
}
bool function::call (value& out, const argument_list& args, state& s)
{
	state::scope scope(s);
	return call(out, args, scope);
}


bool native_function::call (value& out, const argument_list& args, state::scope& scope)
{
	return handle(out, args, scope());
}



argument_list::argument_list (int s)
	: size(s), values(size == 0 ? nullptr : new value[size])
{ }
argument_list::argument_list (const param_list& params)
	: size(params.size()), values(size == 0 ? nullptr : new value[size])
{ }
argument_list::argument_list (const argument_list& other)
	: size(other.size), values(size == 0 ? nullptr : new value[size])
{
	for (int i = 0; i < size; i++)
		values[i] = other.values[i];
}
argument_list::argument_list (std::initializer_list<value> list)
	: size(list.size()), values(size == 0 ? nullptr : new value[size])
{
	int i = 0;
	for (auto val : list)
		values[i++] = val;
}
argument_list::~argument_list ()
{
	delete[] values;
}

value argument_list::get (int i) const
{
	if (i < 0 || i >= size)
		return value();
	else
		return values[i];
}


bool argument_list::check (const std::string& fname, state& s,
				const std::initializer_list<value::value_type>& types, bool err) const
{
	if (size != (int)(types.size()))
	{
		if (err)
			s.error().die()
				<< "Invalid number of arguments supplied to '" << fname
				<< "', expected " << types.size();
		return false;
	}
	int i = 0;
	for (auto t : types)
		if (!values[i++].is_type(t))
		{
			if (err)
				s.error().die()
					<< "Invalid argument #" << i << " to '" << fname
					<< "', expected " << value::type_str(t);
			return false;
		}
	
	return true;
}





int param_list::size() const 
{
	return params.size();
}
int param_list::locate (const std::string& name) const
{
	int i = 0;
	for (auto p : params)
		if (p.name == name)
			return i;
		else
			i++;
	return -1;
}
std::string param_list::param_name (int index) const
{
	return params[index].name;
}
std::shared_ptr<expression> param_list::condition (int index)
{
	return params[index].cond;
}

// let (var : cond) = ...
void param_list::add_param (const std::string& name,
								std::shared_ptr<expression> condition)
{
	params.push_back(param(name, condition));
}
// let (var) = ...
void param_list::add_param (const std::string& name)
{
	params.push_back(param(name, nullptr));//expression::create_true()));
}
// let (value) = ...
void param_list::add_param (std::shared_ptr<expression> right)
{
	int index = params.size();
	auto left = expression::create_closure_ref(index);
	
	add_param("", expression::create_binary(left, right, lexer::token::eql_token));
}

std::shared_ptr<expression> param_list::condition (const std::string& name)
{
	int l = locate(name);
	if (l < 0)
		return nullptr;
	else
		return condition(l);
}
bool param_list::satisfies (bool& out, state::scope& scope)
{
	value val;
	
	for (auto p : params)
		if (p.cond == nullptr)
			continue;
		else if (!p.cond->eval(val, scope))
			return false;
		else if (!val.condition())
		{
			out = false;
			return true;
		}
	out = true;
	return true;
}







soft_function::soft_function (const std::string& n)
	: function(n, false), parent_closure(nullptr)
{ }

soft_function::soft_function (const std::shared_ptr<closure>& scope)
	: function("", false), parent_closure(scope)
{ }

soft_function::~soft_function () {}


void soft_function::add_overload (const std::shared_ptr<func_body>& o)
{
	overloads.push_back(o);
}

bool soft_function::call (value& out, const argument_list& args, state::scope& parent)
{
	state::scope scope(parent(), std::shared_ptr<closure>(new closure(args, parent_closure)));
	std::shared_ptr<expression> to_eval(nullptr);
	
tail_call_recur_point: // if tail call successful, goto here
	
#ifdef XY_REVERSE_OVERLOAD_ORDER
	for (auto it = overloads.crbegin(); it != overloads.crend(); it++)
#else
	for (auto it = overloads.cbegin(); it != overloads.cend(); it++)
#endif
	{
		bool good = false;
		if (!(*it)->params.satisfies(good, scope))
			return false;
		
		if (good)
		{
			to_eval = (*it)->body;
			break;
		}
	}
	
	if (to_eval == nullptr)
	{
		auto& err = parent().error().die();
		err << "No suitable overload for ";
		if (func_name.size() == 0)
			err << "lambda function";
		else
			err << "function '" << func_name << "'";
		err << " found";
		return false;
	}
	else
	{
		expression::tail_call tc(this);
		
		if (!to_eval->eval_tail_call(tc, out, scope))
			return false;
		
		if (tc.do_tail)
		{
			std::shared_ptr<closure> new_closure(new closure(tc.args.size(), parent_closure));
			int i = 0;
			for (auto& v : tc.args)
				new_closure->set(i++, v);
			
			to_eval = nullptr;
			scope.local = new_closure;
			
			goto tail_call_recur_point;
		}
		
		return true;
	}
}



};
