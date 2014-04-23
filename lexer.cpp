#include "include.h"
#include "lexer.h"
#include "state.h"

namespace xy {



lexer::lexer (state& parent_)
	: parent(parent_), line (-1), is_eof(false)
{ }

lexer::~lexer () {}



bool lexer::open (const std::string& filename)
{
	input = inp_stream(new std::fstream(filename, std::ios::in));
	
	if (!input->good())
	{
		parent.error().die()
			<< "Could not read file '" << filename << "'";
		
		return false;
	}
	input_opened();
	return true;
}

bool lexer::open_string (const std::string& data)
{
	input = std::unique_ptr<std::istream>(new std::stringstream(data));
	input_opened();
	return true;
}

void lexer::input_opened ()
{
	line = 1;
	peek_char = '\0';
}





char lexer::read ()
{
	int out;
	
	if (eof())
		out = '\0';
	else if (peek_char == '\0')
	{
		out = input->get();
		
		if (out == '\n')
			line++;
	}
	else
		out = peek_char;
	
	peek_char = '\0';
	if (out == -1)
	{
		is_eof = true;
		return '\0';
	}
	else
		return (char)out;
}
char lexer::peek ()
{
	if (peek_char == '\0')
		peek_char = read();
	return peek_char;
}
bool lexer::eof () const
{
	return input == nullptr ||
		is_eof ||
		(peek_char == '\0' && input->eof());
}
int lexer::line_num () const
{
	return line;
}
const lexer::token& lexer::current () const
{
	return current_token;
}







lexer::token::token (int tok_)
	: tok(tok_) {}

std::string lexer::token::to_str () const
{
	switch (tok)
	{
	case number_token:
		{
			std::stringstream ss;
			ss << num;
			return ss.str();
		}
	case symbol_token:
		return str;
	
	default:
		if (tok < 256)
			return std::string(1, tok);
		else
			return "?";
	}
}














void lexer::trim_left ()
{
	for (;;)
	{
		while (isspace(peek()))
			read();
		
		if (peek() == ';')
			while (read() != '\n')
				;
		else
			break;
	}
}

bool lexer::advance ()
{
	trim_left();
	
	if (isdigit(peek()))
		return parse_num();
	else if (is_sym_char(peek()))
		return parse_sym();
	else
		current_token.tok = read();
	return true;
}

bool lexer::is_sym_char (char c) const
{
	return isdigit(c) || isalpha(c) ||
				c == '_' || c == '#';
}
bool lexer::parse_digit (char c, int base, int& out) const
{
	c = tolower(c);
	if (c >= 'a' && c <= 'f')
	{
		if (base == 16)
			out = 10 + c - 'a';
		else
			return false;
	}
	else if (isdigit(c))
		out = c - '0';
	else
		return false;
	
	return true;
}

bool lexer::parse_num ()
{
	int base = 10, digit;
	number n = 0;
	
	if (peek() == '0')
	{
		read();
		if (peek() == 'x')
		{
			read();
			base = 16;
		}
		// else octal
	}
	
	while (is_sym_char(peek()))
	{
		if (!parse_digit(read(), base, digit))
		{
			parent.error().die_lex(*this) 
				<< "Unexpected token in number literal";
			return false;
		}
		n = (n * base) + digit;
	}
	
	current_token.tok = token::number_token;
	current_token.num = n;
	return true;
}
bool lexer::parse_sym ()
{
	std::stringstream ss;
	while (is_sym_char(peek()))
		ss << read();
	
	current_token.tok = token::symbol_token;
	current_token.str = ss.str();
	return true;
}



};
