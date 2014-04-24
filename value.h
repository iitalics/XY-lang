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
	
	value (value_type type = type_void);
	value (const value& other);
	value& operator=(const value& other);
	
	
	value_type type;
	
	number num;
	bool cond;
	
	
	
	static inline value nil () { return value(type_nil); }
	static value from_number (number n);
	static value from_bool (bool b);
};


};
