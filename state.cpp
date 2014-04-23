#include "include.h"
#include "state.h"
#include "lexer.h"

namespace xy {






state::state ()
	: dead(false)
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
	
	
	if (!lex.advance())
		return false;
		
	while (!lex.current().eof())
	{
		std::cout << "Token: '" << lex.current().to_str() << "'" << std::endl;
		
		
		if (!lex.advance())
			return false;
	} 
	
	
	return true;
}



};