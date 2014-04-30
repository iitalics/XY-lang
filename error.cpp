#include "include.h"
#include "error.h"
#include "lexer.h"

namespace xy {



error_handler::error_handler ()
	: dead(false), error_class(""), line_info("")
{}


void error_handler::dump ()
{
	if (dead)
	{
		std::cerr << "XY " << error_class << "Error" << line_info << ": \n"
		             "    " << die_stream.str() << std::endl;
	}
	
	flush();
}	
void error_handler::flush ()
{
	die_stream.str(std::string());
	dead = false;
}



std::ostream& error_handler::die ()
{
	error_class = "";
	line_info = "";
	
	dead = true;
	return die_stream;
}
std::ostream& error_handler::die_lex (const lexer& lex)
{
	error_class = "Parser ";
	
	if (lex.line_num() != -1)
	{
		std::stringstream ss;
		ss << " [line " << lex.line_num()/* << ", col " << lex.col_num()*/ << "]";
		line_info = ss.str();
	}
	
	dead = true;
	return die_stream;
}


};
