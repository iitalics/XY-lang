#pragma once

namespace xy {


class lexer;

class error_handler
{
public:
	error_handler ();
	
	void dump ();
	std::string flush ();
	
	std::ostream& die ();
	std::ostream& die_lex (const lexer& lex);
	
private:
	bool dead;
	std::stringstream die_stream;
	
	std::string error_class;
	std::string line_info;
};



};
