#include "include.h"
#include "state.h"
#include "lexer.h"
#include "parser.h"

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



};