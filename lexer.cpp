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
	input = std::unique_ptr<std::istream>(new std::istringstream(data));
	input_opened();
	return true;
}

void lexer::input_opened ()
{
	line = 1;
	peek_char = '\0';
}





//// character streams

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
		else if (out == -1)
		{
			out = '\0';
			is_eof = true;
		}
	}
	else
		out = peek_char;
	
	peek_char = '\0';
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
		(is_eof && peek_char == '\0');
}
int lexer::line_num () const
{
	return line;
}
const lexer::token& lexer::current () const
{
	return current_token;
}









//// token parsing


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
	
	char chr = read();
	current_token.tok = chr;
	
	for (auto tc : token::two_chars)
		if (tc.a == chr && tc.b == peek())
		{
			read();
			current_token.tok = tc.tok;
			return true;
		}
	
	return true;
}

bool lexer::is_sym_char (char c)
{
	return isdigit(c) || isalpha(c) ||
				c == '_' || c == '#';
}
bool lexer::parse_digit (char c, int base, int& out)
{
	c = tolower(c);
	if (isalpha(c))
		out = 10 + c - 'a';
	else if (isdigit(c))
		out = c - '0';
	else
		goto fail;
	
	if (out >= base)
		goto fail;
	
	return true;
fail:
	parent.error().die_lex(*this) 
		<< "Unexpected token in number literal";
	return false;
}

bool lexer::parse_num ()
{
	int digit, base = 10;
	number mag, n = 0;
	
	if (peek() == '0')
	{
		read();
		if (peek() == 'x')
		{
			read();
			base = 16;
		}
		else if (isdigit(peek()))
			base = 8;
	}
	
	while (is_sym_char(peek()))
	{
		if (!parse_digit(read(), base, digit))
			return false;
		
		n = (n * base) + digit;
	}
	if (peek() == '.')
	{
		read();
		
		if (base != 10)
		{
			parent.error().die_lex(*this)
				<< "Non-base 10 decimals not supported";
			return false;
		}
		mag = 1;
		
		while (is_sym_char(peek()))
		{
			if (!parse_digit(read(), base, digit))
				return false;
			
			mag /= base;
			n += mag * digit;
		}
	}
	
	current_token.tok = token::number_token;
	current_token.num = n;
	return true;
}
bool lexer::parse_sym ()
{
	std::ostringstream ss;
	while (is_sym_char(peek()))
		ss << read();
	
	for (int i = token::keywords.size(); i-- > 0; )
		if (ss.str() == token::keywords[i])
		{
			current_token.tok = token::keyword__start + i;
			return true;
		}
	
	current_token.tok = token::symbol_token;
	current_token.str = ss.str();
	return true;
}










//// token

lexer::token::token (int tok_)
	: tok(tok_) {}

std::string lexer::token::to_str () const
{
	switch (tok)
	{
	case number_token:
		{
			std::ostringstream ss;
			ss << num;
			return ss.str();
		}
	case symbol_token:
		return "symbol " + str;
	
	default:
		if (tok < 256)
			return std::string(1, tok);
		else if (tok >= keyword__start)
			return "keyword " + keywords[tok - keyword__start];
		else
		{
			for (auto tc : two_chars)
				if (tc.tok == tok)
					return tc.str();
			return "?";
		}
	}
}


std::vector<std::string> lexer::token::keywords
{
	"let", "struct", "true", "false", "or", "and"
};

std::vector<lexer::token::two_char> lexer::token::two_chars =
{
	two_char("..", seq_token),
	two_char("==", eql_token),
	two_char("!=", neq_token),
	two_char(">=", gre_token),
	two_char("<=", lse_token)
};

std::string lexer::token::two_char::str () const
{
	char buf[3];
	buf[0] = a;
	buf[1] = b;
	buf[2] = '\0';
	return std::string(buf);
}



};
