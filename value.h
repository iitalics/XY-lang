#pragma once
#include "state.h"

namespace xy {

class function;
class error_handler;
class list;
struct argument_list;

struct value
{
	enum value_type
	{
		type_void = 0,
		type_nil,
		type_number,
		type_bool,
		type_function,
		type_list,
		type_string
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
	std::shared_ptr<function> func_obj;
	std::shared_ptr<list> list_obj;
	std::string str;
	
	
	bool condition () const;
	int integer () const;
	
	bool apply_operator (value& out, int op, const value& other, state& parent);
	bool apply_unary (value& out, int op, state& parent);
	comparison compare (const value& other, state& parent);
	
	inline bool equals (const value& other, state& parent)
	{
		return compare(other, parent) & compare_equal;
	}
	
	
	bool call (value& out, const argument_list& args, state& parent);
	
	std::string to_str () const;
	std::string type_str () const;
	
	
	
	
	
	
	static inline value nil () { return value(type_nil); }
	static value from_number (number n);
	static value from_bool (bool b);
	static value from_function (std::shared_ptr<function> f);
	static value from_list (std::shared_ptr<list> l);
	static value from_string (const std::string& str);
	
	static std::string type_str (value_type t);
	static std::string true_string ();
	static std::string false_string ();
};


};
