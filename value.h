#pragma once
#include "state.h"

namespace xy {

class function;
class error_handler;
struct argument_list;

struct value
{
	enum value_type
	{
		type_void = 0,
		type_nil,
		type_number,
		type_bool,
		type_function
		/*type_list*/
	};
	enum comparison
	{
		compare_none = 0,
		compare_equal = 1,
		compare_greater = 1 << 1
	};
	
	value (value_type type = type_void);
	value (const value& other);
	value& operator=(const value& other);
	
	
	value_type type;
	
	
	union
	{
		number num;
		bool cond;
	};
	std::shared_ptr<function> func;
	
	
	bool condition () const;
	
	bool apply_operator (value& out, int op, const value& other, error_handler& error);
	bool apply_unary (value& out, int op, error_handler& error);
	bool call (value& out, const argument_list& args, state::scope& scope);
	comparison compare (const value& other);
	
	std::string to_str () const;
	std::string type_string () const;
	
	
	
	
	
	
	static inline value nil () { return value(type_nil); }
	static value from_number (number n);
	static value from_bool (bool b);
	static value from_function (std::shared_ptr<function> f);
	
	static std::string type_string (value_type t);
	static std::string true_string ();
	static std::string false_string ();
};


};
