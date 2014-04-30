#include "include.h"
#include "value.h"
#include "lexer.h"
#include "function.h"
#include "list.h"

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
	if (type == type_function)
		func_obj = other.func_obj;
	if (type == type_list)
		list_obj = other.list_obj;
}

value& value::operator=(const value& other)
{
	type = other.type;
	if (type == type_number)
		num = other.num;
	if (type == type_bool)
		cond = other.cond;
	if (type == type_function)
		func_obj = other.func_obj;
	if (type == type_list)
		list_obj = other.list_obj;
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
		return cond ? true_string() : false_string();
		
	case type_function:
		if (func_obj->is_lambda())
			return "<lambda function>";
		else
		{
			ss << "<";
			if (func_obj->is_native())
				ss << "native ";
			ss << "function '" << func_obj->name() << "'>";
			return ss.str();
		}
		
	case type_list:
		{
			int i, size = list_obj->size();
			ss << "[";
			for (i = 0; i < size; i++)
				ss << ((i == 0) ? " " : ", ")
				   << list_obj->get(i).to_str();
			ss << " ]";
			return ss.str();
		}
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
value value::from_function (std::shared_ptr<function> f)
{
	value v(type_function);
	v.func_obj = f;
	return v;
}
value value::from_list (std::shared_ptr<list> l)
{
	value v(type_list);
	v.list_obj = l;
	return v;
}


bool value::apply_operator (value& out, int op, const value& other, state& parent)
{
	switch (op)
	{
	case lexer::token::eql_token:
		out = from_bool(compare(other, parent) & compare_equal);
		return true;
	
	case lexer::token::neq_token:
		out = from_bool(!(compare(other, parent) & compare_none));
		return true;
	
	case '.':
		if (type == type_list &&
				other.type == type_number)
		{
			out = list_obj->get((int)(other.num));
			return true;
		}
		break;
	
	case lexer::token::seq_token:
		if (type == type_list &&
				other.type == type_number)
		{
			out = value::from_list(std::shared_ptr<list>(
						new list_sublist(list_obj, (int)(other.num))));
			return true;
		}
		break;
	
	case '+':
		if (type == type_list &&
				other.type == type_list)
		{
			out = value::from_list(std::shared_ptr<list>(
					new list_concat(list_obj, other.list_obj)));
			return true;
		}
	
	default: break;
	}
	
	if (type != type_number ||
			other.type != type_number)
		goto bad_input;
	
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
		if (other.num == 0) // obligatory
		{
			parent.error().die()
				<< "Cannot divide by zero";
			return false;
		}
		out.num = num / other.num;
		break;
	case '^':
		out.num = pow(num, other.num);
		break;
		
	case '>':
		out = from_bool(compare(other, parent) & compare_greater);
		break;
		
	case '<':
		out = from_bool(!(compare(other, parent) & (compare_greater | compare_equal)));
		break;
		
	case lexer::token::gre_token: // '>='
		out = from_bool(compare(other, parent) & (compare_greater | compare_equal));
		break;
	
	case lexer::token::lse_token: // '<='
		out = from_bool(!(compare(other, parent) & compare_greater));
		break;
	
	default:
		goto bad_input;
	}
	
	return true;
	
bad_input:
	parent.error().die()
		<< "Cannot apply operator '" << lexer::token(op).to_str()
		<< "' to values of type '" << type_string() << "' and '" << other.type_string() << "'";
	return false;
}


bool value::apply_unary (value& out, int op, state& parent)
{
	switch (op)
	{
	case '-':
		if (type != type_number)
			goto bad_input;
		out.num = -num;
		out.type = type_number;
		return true;
		
	case '!':
		out.cond = !condition();
		out.type = type_bool;
		return true;
	
	default: break;
	}
bad_input:
	parent.error().die()
		<< "Cannot apply unary operator '" << lexer::token(op).to_str() 
		<< "' to value of type '" << type_string() << "'";
	return false;
}

value::comparison value::compare (const value& other, state& parent)
{
	if (type != other.type)
		return compare_none;
	
	switch (type)
	{
	case type_number:
		if (num == other.num)
			return compare_equal;
		if (num > other.num)
			return compare_greater;
		return compare_none;
	
	case type_bool:
		return cond == other.cond ? compare_equal : compare_none;
	
	case type_void: case type_nil:
		return compare_equal;
		
	case type_list:
		return list_obj->equals(other.list_obj, parent) ? compare_equal : compare_none;
		
	default:
		return compare_none;
	};
}


bool value::call (value& out, const argument_list& args, state& parent)
{
	if (type == type_number && args.size == 1)
	{
		return apply_operator(out, '*', args.values[0], parent);
	}
	if (type != type_function)
	{
		parent.error().die()
			<< "Cannot apply non-function value '" << to_str() << "'";
		return false;
	}
	state::scope scope(parent);
	
	return func_obj->call(out, args, scope);
}


bool value::condition () const
{
	switch (type)
	{
	case type_number:
		return num != 0;
	case type_bool:
		return cond;
	case type_void: case type_nil:
		return false;
	default:
		return true;
	}
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
	case type_list: return "list";
	default: return "??";
	}
}
std::string value::true_string () { return "true"; }
std::string value::false_string () { return "false"; }


};
