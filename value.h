#pragma once

namespace xy {


struct value
{
	enum value_type
	{
		type_void = 0,
		type_nil,
		type_number,
		type_bool
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
	
	number num;
	bool cond;
	
	
	bool condition () const;
	
	bool apply_operator (value& out, int op, const value& other);
	comparison compare (const value& other);
	
	std::string to_str () const;
	std::string type_string () const;
	
	
	
	
	
	
	static inline value nil () { return value(type_nil); }
	static value from_number (number n);
	static value from_bool (bool b);
	
	static std::string type_string (value_type t);
};


};
