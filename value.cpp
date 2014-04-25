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

std::string value::to_str () const
{
	std::ostringstream ss;
	
	switch (type)
	{
	case type_void:
		return "void";
		
	case type_nil:
		return "nil";
	
	case type_number:
		ss << num;
		return ss.str();
		
	case type_bool:
		return cond ? "true" : "false";
		
	default:
		return "??";
	}
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



bool value::apply_operator (value& out, int op, const value& other)
{
	if (type != type_number ||
			other.type != type_number)
		return false;
	
	out.type = type_number;
	switch (op)
	{
	case '+':
		out.num = num + other.num;
		break;
	case '-':
		out.num = num - other.num;
		break;
	case '*':
		out.num = num * other.num;
		break;
	case '/':
		out.num = num / other.num;
		break;
	case '^':
		out.num = pow(num, other.num);
		break;
	default:
		return false;
	}
	
	return true;
}


std::string value::type_string () const
{
	return type_string(type);
}
std::string value::type_string (value_type t)
{
	switch (t)
	{
	case type_number: return "number";
	case type_void: return "void";
	case type_nil: return "nil";
	case type_bool: return "boolean";
	default: return "??";
	}
}


};
