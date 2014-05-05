#include "include.h"
#include "environment.h"
#include "state.h"
#include "function.h"

namespace xy {


environment::environment (state& p)
	: parent(p)
{ }

environment::~environment () {}



std::shared_ptr<function> environment::find_function (const std::string& name)
{
	for (auto f : funcs)
		if (f->name() == name)
			return f;
	return nullptr;
}
void environment::add_function (const std::shared_ptr<function>& func)
{
	funcs.push_back(func);
}


std::shared_ptr<soft_function> environment::find_or_add (const std::string& name)
{
	auto func(find_function(name));
	
	if (func == nullptr)
	{
		std::shared_ptr<soft_function> soft_func(new soft_function(name));
		add_function(soft_func);
		return soft_func;
	}
	else if (!func->is_native())
		return std::static_pointer_cast<soft_function>(func);
	else
		return nullptr;
}


};
