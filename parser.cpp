#include "include.h"
#include "parser.h"
#include "lexer.h"
#include "state.h"

namespace xy {


parser::parser (state& p, lexer& l)
	: parent(p), lex(l)
{
}

parser::~parser ()
{
}




bool parser::parse_env (environment& env)
{
	return true;
}




};
