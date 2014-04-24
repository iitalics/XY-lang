#pragma once

namespace xy {

class state;
class lexer;
class environment;

class parser
{
public:
	parser (state& parent, lexer& lex);
	~parser ();
	
	bool parse_env (environment& env);
	
private:
	state& parent;
	lexer& lex;
};



};
