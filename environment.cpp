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



};
