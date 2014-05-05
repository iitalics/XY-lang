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
	
	e.add_native("length", [] ( _args_ )
	{
		if (!args.check("length", s, { value::type_iterable }))
			return false;
		
		out = value::from_number(args.get(0).list_size());
		return true;
	});
	
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
	
	e.add_native("foldr", [] ( _args_ )
	{
		if (!args.check("foldr", s, { value::type_function,
		                              value::type_any,
									  value::type_iterable }))
			return false;
		
		auto func(args.get(0).func_obj);
		value z(args.get(1));
		value it(args.get(2));
		
		for (int i = it.list_size(); i-- > 0; )
		{
			value x(it.list_get(i));
			
			if (!func->call(z, argument_list { z, x }, s))
				return false;
		}
		
		out = z;
		return true;
	});
	
	e.add_native("fread", [] ( _args_ )
	{
		if (!args.check("fread", s, { value::type_string,
		                              value::type_function,
									  value::type_function }))
			return false;
		
		std::ifstream fs(args.get(0).str);
		if (fs.good())
		{
			fs.seekg (0, std::ios::end);
			int len = fs.tellg();
			fs.seekg (0, std::ios::beg);
			
			char* buf = new char[len];
			fs.read(buf, len);
			fs.close();
			
			std::string str(buf, len);
			delete[] buf;
			
			return args.get(1).func_obj->call(out, argument_list
					{
						value::from_string(str)
					}, s);
		}
		else
			return args.get(2).func_obj->call(out, argument_list(), s);
	});
	
	e.add_native("fwrite", [] ( _args_ ) 
	{
		if (!args.check("fwrite", s, { value::type_string,
		                              value::type_function,
									  value::type_function }))
			return false;
		
		std::ofstream fs(args.get(0).str);
		bool success = false;
		if (fs.good())
		{
			value data;
			if (!args.get(1).func_obj->call(data, argument_list(), s))
				return false;
			fs << data.to_str();
			fs.close();
			success = true;
		}
		
		return args.get(2).func_obj->call(out, argument_list
				{
					value::from_bool(success)
				}, s);
	});
	
	e.add_native("display", [] ( _args_ )
	{
		for (int i = 0; i < args.size; i++)
			std::cout << args.get(i).to_str();
		return true;
	});
}




};