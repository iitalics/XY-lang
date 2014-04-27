#include "include.h"
#include "function.h"
#include "value.h"

namespace xy {


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
	: function(n, true)
{ }

soft_function::~soft_function () {}


void soft_function::add_overload (const std::shared_ptr<func_body>& o)
{
	overloads.push_back(o);
}

bool soft_function::call (value& out, const argument_list& args, state::scope& parent)
{
	state::scope scope(parent(), std::shared_ptr<closure>(new closure(args)));
	
	for (auto overload : overloads)
	{
		return overload->body->eval(out, scope);
	}
	
	parent().error().die()
		<< "No suitable overload for function '" << func_name << "' found";
	return false;
}




soft_function::func_body::func_body (std::shared_ptr<expression> e)
	: body(e)
{ }


};
