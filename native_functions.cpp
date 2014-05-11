#include "include.h"
#include "environment.h"
#include "function.h"
#include "value.h"
#include "list.h"

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
//

#define check_one(name_) \
	if (!args.check(name_, s, { value::type_any } )) return false

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
	
	e.add_native("indexof", [] ( _args_ )
	{
		if (!args.check("indexof", s, { value::type_iterable,
                                           value::type_any }))
			return false;
		
		auto it(args.get(0));
		value search(args.get(1));
		
		for (int i = 0, size = it.list_size(); i < size; i++)
			if (it.list_get(i).equals(search, s))
			{
				out = value::from_number(i);
				return true;
			}
		out = value::from_number(-1);
		return true;
	});
	
	e.add_native("distribute", [] ( _args_ )
	{
		if (!args.check("distribute", s, { value::type_function,
		                              value::type_iterable }))
			return false;
		
		std::vector<value> a, b;
		auto func(args.get(0).func_obj);
		auto it(args.get(1));
		value r, v;
		for (int i = 0, size = it.list_size(); i < size; i++)
		{
			v = it.list_get(i);
			if (!func->call(r, argument_list { v }, s))
				return false;
			(r.condition() ? a : b).push_back(v);
		}
		
		// not really happy with this
		out = value::from_list(list::basic(std::vector<value> {
				value::from_list(list::basic(a)),
				value::from_list(list::basic(b)),
			}));
		return true;
	});
	
	e.add_native("first", [] ( _args_ )
	{
		if (!args.check("first", s, { value::type_function,
		                              value::type_iterable,
									  value::type_any }))
			return false;
		
		auto func(args.get(0).func_obj);
		auto it(args.get(1));
		value r;
		for (int i = 0, size = it.list_size(); i < size; i++)
			if (!func->call(r, argument_list { it.list_get(i) }, s))
				return false;
			else if (r.condition())
			{
				out = r;
				return true;
			}
		
		out = args.get(2);
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
	
	
	e.add_native("filter", [] ( _args_ )
	{
		if (!args.check("filter", s, { value::type_function,
		                               value::type_iterable }))
			return false;
		std::vector<value> vs;
		auto func(args.get(0).func_obj);
		value it(args.get(1));
		value x, b;
		for (int i = 0, size = it.list_size(); i < size; i++)
		{
			x = it.list_get(i);
			
			if (!func->call(b, argument_list { x }, s))
				return false;
			
			if (b.condition())
				vs.push_back(x);
		}
		
		out = value::from_list(list::basic(vs));
		return true;
	});
	
	
	e.add_native("map", [] ( _args_ )
	{
		if (!args.check("map", s, { value::type_function,
		                               value::type_iterable }))
			return false;
		std::vector<value> vs;
		auto func(args.get(0).func_obj);
		value it(args.get(1));
		value x;
		for (int i = 0, size = it.list_size(); i < size; i++)
		{
			x = it.list_get(i);
			
			if (!func->call(x, argument_list { x }, s))
				return false;
			
			vs.push_back(x);
		}
		
		out = value::from_list(list::basic(vs));
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
			std::string line;
			std::ostringstream ss;
			
			while (!fs.eof())
			{
				std::getline(fs, line);
				ss << line << std::endl;
			}
			
			return args.get(1).func_obj->call(out, argument_list
					{
						value::from_string(ss.str())
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
	
	e.add_native("die", [] ( _args_ )
	{
		if (!args.check("die", s, { value::type_string }))
			return false;
		
		s.error().die()
			<< args.get(0).str;
		return false;
	});
	e.add_native("try", [] ( _args_ )
	{
		if (!args.check("fwrite", s, { value::type_function, value::type_function }))
			return false;
		
		if (!args.get(0).func_obj->call(out, argument_list(), s))
		{
			std::string msg(s.error().flush());
			
			return args.get(1).func_obj->call(out, argument_list
				{
					value::from_string(msg)
				}, s);
		}
		else
			return true;
	});
	
	
	///-    data types    -///
	
	e.add_native("int", [] ( _args_ )
	{
		check_one("int");
		value v(args.get(0));
		if (v.type == value::type_number)
			out = value::from_number((int)(v.num));
		else if (v.type == value::type_string)
			out = value::from_number(v.str.c_str()[0]);
		else
			out = value::from_number(0);
		return true;
	});
	e.add_native("string", [] ( _args_ )
	{
		check_one("string");
		out = value::from_string(args.get(0).to_str());
		return true;
	});
	e.add_native("number", [] ( _args_ )
	{
		check_one("number"); 
		value v(args.get(0));
		if (v.type == value::type_number) out = v;
		else if (v.type == value::type_string)
		{
			std::istringstream ss(v.str);
			number n;
			ss >> n;
			out = value::from_number(n);
		}
		else
			out = value::from_number(0);
		return true;
	});
	e.add_native("list", [] ( _args_ ) // not sure why the fuck you'd ever use this function
	{
		std::vector<value> q;
		for (int i = 0; i < args.size; i++)
			q.push_back(args.values[i]);
		
		out = value::from_list(list::basic(q));
		return true;
	});
	e.add_native("bool", [] ( _args_ )
	{
		check_one("bool"); 
		out = value::from_bool(args.get(0).condition());
		return true;
	});
	e.add_native("void", [] ( _args_ )
	{
		out = value();
		return true;
	});
	
	#define type_check_func(name_, v_) \
		e.add_native(name_, [] ( _args_ ) { \
			check_one(name_); \
			out = value::from_bool(args.get(0).is_type(value:: v_ )); \
			return true; \
		})
	
	
	type_check_func("void?", type_void);
	type_check_func("list?", type_list);
	type_check_func("string?", type_string);
	type_check_func("number?", type_number);
	type_check_func("function?", type_function);
	type_check_func("map?", type_map);
	type_check_func("int?", type_int);
	type_check_func("iterable?", type_iterable);
	type_check_func("orderable?", type_orderable);
}




};