#include "include.h"
#include "value.h"

namespace xy {


	
value::value (value_type t)
	: type(t)
{}

value::value (const value& other)
	: type(other.type)
{
	if (type == type_number)
		num = other.num;
	if (type == type_bool)
		cond = other.cond;
}

value& value::operator=(const value& other)
{
	type = other.type;
	if (type == type_number)
		num = other.num;
	if (type == type_bool)
		cond = other.cond;
	return *this;
}


value value::from_number (number n)
{
	value v(type_number);
	v.num = n;
	return v;
}

value value::from_bool (bool b)
{
	value v(type_bool);
	v.cond = b;
	return v;
}



};
