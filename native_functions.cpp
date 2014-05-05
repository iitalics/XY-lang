#include "include.h"
#include "environment.h"
#include "function.h"
#include "value.h"

namespace xy {







#define _args_ \
	\
	value& out, const argument_list& args, state& s

#define math_func1(name_, func_)                                          \
	e.add_native(name_, [] ( _args_ ) {                                  \
		if (!args.check(name_, s, { value::type_number })) return false; \
		out = value::from_number( func_ (args.get(0).num));              \
		return true;                                                     \
	})

void state::import_native_functions (environment& e)
{
	math_func1("sqrt", sqrt);
	math_func1("log", log);
	math_func1("sin", sin);
	math_func1("cos", cos);
	math_func1("tan", tan);
	
	
	e.add_native("foldl", [] ( _args_ )
	{
		if (!args.check("foldl", s, { value::type_function,
		                              value::type_any,
									  value::type_iterable }))
			return false;
		
		auto func(args.get(0).func_obj);
		value z(args.get(1));
		value it(args.get(2));
		
		for (int i = 0, size = it.list_size(); i < size; i++)
		{
			value x(it.list_get(i));
			
			if (!func->call(z, argument_list { z, x }, s))
				return false;
		}
		
		out = z;
		return true;
	});
}




};