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
	switch (type)
	{
	case type_list:		list_obj = other.list_obj; break;
	case type_bool:		cond = other.cond; break;
	case type_string:	str = other.str; break;
	case type_number: 	num = other.num; break;
	case type_function:	func_obj = other.func_obj; break;
	default: break;
	}
}

value& value::operator=(const value& other)
{
	switch (type = other.type)
	{
	case type_list:		list_obj = other.list_obj; break;
	case type_bool:		cond = other.cond; break;
	case type_string:	str = other.str; break;
	case type_number: 	num = other.num; break;
	case type_function:	func_obj = other.func_obj; break;
	default: break;
	}
	return *this;
}

std::string value::to_str () const
{
	std::ostringstream ss;
	
	switch (type)
	{
	case type_void:
		return "void";
		
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
			{
				value v(list_obj->get(i));
				std::string s(v.to_str());
				if (v.is_type(type_string))
					s = "\"" + s + "\"";
				
				ss << ((i == 0) ? " " : ", ") << s;
			}
			ss << " ]";
			return ss.str();
		}
	
	case type_string:
		return str;
	
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
value value::from_string (const std::string& str)
{
	value v(type_string);
	v.str = str;
	return v;
}


bool value::apply_operator (value& out, int op, const value& other, state& parent)
{
	if (type == type_void)
		switch (op)
		{
		case '+':
			out = other;
			return true;
			
		case '-': case '/': case '*':
		case '^': case '.': case lexer::token::seq_token:
			out = value();
			return true;
		default: break;
		}
	
	switch (op)
	{
	case lexer::token::eql_token:
		out = from_bool(compare(other, parent) & compare_equal);
		return true;
	
	case lexer::token::neq_token:
		out = from_bool(!(compare(other, parent) & compare_none));
		return true;
		
	case '.':
		if (is_type(type_iterable) && other.is_type(type_int))
		{
			out = list_get((int)(other.num));
			return true;
		}
		break;
	
	case lexer::token::seq_token:
		if (is_type(type_list) &&
				other.is_type(type_int))
		{
			int index(other.num);
			if (index < 0)
			{
				parent.error().die() 
					<< "Cannot access negative list index";
				return false;
			}
			
			out = value::from_list(list::sublist(list_obj, index));
			return true;
		}
		if (is_type(type_string) &&
				other.is_type(type_int))
		{
			int index(other.num);
			int size(str.size());
			if (index < 0)
			{
				parent.error().die() 
					<< "Cannot access negative string index";
				return false;
			}
			
			if (index >= size)
				out = value::from_string("");
			else
				out = value::from_string(str.substr(index));
			return true;
		}
		if (is_type(type_int) && other.is_type(type_int))
		{
			int start(num);
			int end(other.num);
			std::vector<value> vs;
			for (int i = start; i <= end; i++)
				vs.push_back(value::from_number(i));
			out = value::from_list(list::basic(vs));
			return true;
		}
		break;
	
	case '+':
		if (is_type(type_list) && other.is_type(type_list))
		{
			out = value::from_list(list::concat(list_obj, other.list_obj));
			return true;
		}
		if (is_type(type_string))
		{
			std::ostringstream ss;
			ss << str;
			ss << other.to_str();
			out = value::from_string(ss.str());
			return true;
		}
		break;
	
	default: break;
	}
	
	if (type == other.type &&
			is_type(type_orderable))
		switch (op)
		{
		case '>':
			out = from_bool(compare(other, parent) & compare_greater);
			return true;
			
		case '<':
			out = from_bool(!(compare(other, parent) & (compare_greater | compare_equal)));
			return true;
			
		case lexer::token::gre_token: // '>='
			out = from_bool(compare(other, parent) & (compare_greater | compare_equal));
			return true;
		
		case lexer::token::lse_token: // '<='
			out = from_bool(!(compare(other, parent) & compare_greater));
			return true;
		
		default: break;
		}
	
	if (!(is_type(type_number) && other.is_type(type_number)))
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
	
	default:
		goto bad_input;
	}
	
	return true;
	
bad_input:
	parent.error().die()
		<< "Cannot apply operator '" << lexer::token(op).to_str()
		<< "' to values of type '" << type_str() << "' and '" << other.type_str() << "'";
	return false;
}


bool value::apply_unary (value& out, int op, state& parent)
{
	switch (op)
	{
	case '-':
		if (!is_type(type_number))
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
		<< "' to value of type '" << type_str() << "'";
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
	
	case type_void:
		return compare_equal;
		
	case type_list:
		return list_obj->equals(other.list_obj, parent) ? compare_equal : compare_none;
		
	case type_string:
		return (str == other.str) ? compare_equal : compare_none;
		
	default:
		return compare_none;
	};
}


bool value::call (value& out, const argument_list& args, state& parent)
{
	if (args.size == 1)
	{
		if (is_type(type_number) && args.size == 1)
		{
			return apply_operator(out, '*', args.values[0], parent);
		}
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

value value::list_get (int i)
{
	if (type == type_list)
		return list_obj->get(i);
	else if (type == type_string)
	{
		if (i >= int(str.size()))
			return value::from_string("");
		else
			return value::from_string(str.substr(i, 1));
	}
	else
		return value();
}
int value::list_size ()
{
	if (type == type_list)
		return list_obj->size();
	else if (type == type_string)
		return (int)(str.size());
	else
		return 0;
}

bool value::condition () const
{
	switch (type)
	{
	case type_number:
		return num != 0;
	case type_bool:
		return cond;
	case type_list:
		return list_obj != list::empty();
	case type_string:
		return str.size() > 0;
	case type_void:
		return false;
	default:
		return true;
	}
}

bool value::is_type (value_type t) const
{
	if (t == type_any)
		return true;
	
	if (t == type_int)
		return type == type_number &&
			(num == (int)num);
	
	if (t == type_iterable)
		return type == type_list || type == type_string;
	
	if (t == type_orderable)
		return type == type_number || type == type_string;
	
	return type == t;
}




std::string value::type_str () const
{
	if (is_type(type_int))
		return type_str(type_int);
	else
		return type_str(type);
}
std::string value::type_str (value_type t)
{
	switch (t)
	{
	case type_number: return "number";
	case type_void: return "void";
	case type_bool: return "bool";
	case type_list: return "list";
	case type_function: return "function";
	
	case type_int: return "integer";
	case type_iterable: return "iterable";
	case type_orderable: return "orderable";
	case type_any: return "any";
	default: return "??";
	}
}
std::string value::true_string () { return "true"; }
std::string value::false_string () { return "false"; }


};
