#include "include.h"
#include "state.h"
#include "lexer.h"
#include "parser.h"
#include "value.h"
#include "parser.h"
#include "function.h"

namespace xy {






state::state ()
	: global_env(*this)
{
}


state::~state ()
{
}


bool state::load (const std::string& filename)
{
	lexer lex(*this);
	
	if (!lex.open(filename))
		return false;
	
	parser parse(parser(*this, lex));
	
	if (!parse.parse_env(global_env))
		return false;
	
	if (!lex.current().eof())
		return lex.unexpect();
	
	// TODO: invoke main()
	
	return true;
}




closure::closure (int s, const std::shared_ptr<closure>& p)
	: parent(p), closure_size(s), values(new value[closure_size])
{ }

closure::closure (const argument_list& args, const std::shared_ptr<closure>& p)
	: parent(p), closure_size(args.size), values(new value[closure_size])
{
	for (int i = 0; i < args.size; i++)
		values[i] = args.values[i];
}
closure::~closure ()
{
	delete[] values;
}
value closure::get (int index, int depth)
{
	if (depth > 0)
	{
		if (parent == nullptr)
			return value();
		else
			return parent->get(index, depth - 1);
	}
	if (index < 0 || index >= closure_size)
		return value();
	
	return values[index];	
}
bool closure::set (int index, const value& val)
{
	if (index < 0 || index >= closure_size)
		return false;
	
	values[index] = val;
	return true;
}
int closure::size () const
{
	return closure_size;
}

};