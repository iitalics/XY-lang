#include "include.h"
#include "function.h"
#include "value.h"

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

// let (var : cond) = ...
void param_list::add_param (const std::string& name,
								std::shared_ptr<expression> condition)
{
	params.push_back(param(name, condition));
}
// let (var) = ...
void param_list::add_param (const std::string& name)
{
	params.push_back(param(name, expression::create_true()));
}
// let (value) = ...
void param_list::add_param (std::shared_ptr<expression> a)
{
}

std::shared_ptr<expression> param_list::condition (int index)
{
	return nullptr;
}
std::shared_ptr<expression> param_list::condition (const std::string& name)
{
	int l = locate(name);
	if (l < 0)
		return nullptr;
	else
		return condition(l);
}







soft_function::soft_function (const std::string& n)
	: function(n, false)
{ }

soft_function::~soft_function () {}


void soft_function::add_overload (const std::shared_ptr<func_body>& o)
{
	overloads.push_back(o);
}

bool soft_function::call (value& out, const argument_list& args, state::scope& parent)
{
	state::scope scope(parent(), std::shared_ptr<closure>(new closure(args)));
	
#ifdef XY_REVERSE_OVERLOAD_ORDER
	for (auto it = overloads.crbegin(); it != overloads.crend(); it++)
#else
	for (auto it = overloads.cbegin(); it != overloads.cend(); it++)
#endif
	{
		return (*it)->body->eval(out, scope);
	}
	
	auto& err = parent().error().die();
	err << "No suitable overload for ";
	if (func_name.size() == 0)
		err << "lambda function";
	else
		err << "function '" << func_name << "'";
	err << " found";
	return false;
}



};
