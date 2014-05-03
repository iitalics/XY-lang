#include "include.h"
#include "lexer.h"
#include "state.h"
#include "value.h"
#include "syntax.h"

namespace xy {




std::vector<std::string> lexer::token::keywords
{
	"let", "with",
	
	value::true_string(), value::false_string(),
	
	"or", "and"
};

std::vector<lexer::token::two_char> lexer::token::two_chars =
{
	two_char("..", seq_token),
	two_char("==", eql_token),
	two_char("!=", neq_token),
	two_char(">=", gre_token),
	two_char("<=", lse_token)
};




lexer::lexer (state& parent_)
	: parent(parent_), is_eof(false),
	  line (-1), col(-1), col_display(-1),
	  file_name("")
{ }

lexer::~lexer () {}



bool lexer::open (const std::string& filename)
{
	file_name = filename;
	
	input = inp_stream(new std::fstream(filename, std::ios::in));
	
	if (!input->good())
	{
		parent.error().die()
			<< "Could not read file '" << filename << "'";
		
		return false;
	}
	return input_opened();
}

bool lexer::open_string (const std::string& data)
{
	input = std::unique_ptr<std::istream>(new std::istringstream(data));
	return input_opened();
}

bool lexer::input_opened ()
{
	line = 1;
	col = col_display = 0;
	peek_char = '\0';
	
	return advance();
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
		{
			line++;
			col = 0;
		}
		else if (out == -1)
		{
			out = '\0';
			is_eof = true;
		}
		else
			col++;
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

int lexer::line_num () const { return line; }
int lexer::col_num () const { return col_display; }
const lexer::token& lexer::current () const { return current_token; }
std::string lexer::file () const { return file_name; }



//// errors
bool lexer::expect (int tok, bool adv)
{
	if (current_token.tok != tok)
	{
		std::ostream& ss = parent.error().die_lex(*this);
		
		ss << "Expected '";
		if (tok == token::number_token)
			ss << token::number_string();
		else if (tok == token::symbol_token)
			ss << token::symbol_string();
		else
			ss << token(tok).to_str();
		
		ss << "', got '" << current_token.to_str() << "'";
		return false;
	}
	else if (adv)
		return advance();
	else
		return true;
}
bool lexer::unexpect ()
{
	parent.error().die_lex(*this) 
		<< "Unexpected token '" << current_token.to_str() << "'";
	return false;
}







//// token parsing


void lexer::trim_left ()
{
	for (;;)
	{
		while (isspace(peek()))
			read();
		
		if (peek() == ';')
			while (!eof() && read() != '\n')
				;
		else
			break;
	}
}

bool lexer::advance ()
{
	trim_left();
	
	col_display = col;
	
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
				c == '_' || c == '#' || c == '?';
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
	case eof_token:
		return eof_string();
		
	case number_token:
		{
			std::ostringstream ss;
			ss << num;
			return ss.str();
		}
	case symbol_token:
		return str;
	
	default:
		if (tok < 256)
			return std::string(1, tok); // char -> string
		else if (tok >= keyword__start)
			return keywords[tok - keyword__start];
		else
			for (auto tc : two_chars)
				if (tc.tok == tok)
					return tc.str();
		return "?";
	}
}

bool lexer::token::is_unary_op () const
{
	return tok == '-' || tok == '!';
}
bool lexer::token::is_binary_op () const
{
	return tok == '>' || tok == '<' ||
		tok == '+' || tok == '-' ||
		tok == '*' || tok == '/' ||
		tok == '^' || tok == '.' ||
		tok == seq_token || tok == eql_token ||
		tok == neq_token || tok == gre_token ||
		tok == lse_token ||
		tok == keyword_and || tok == keyword_or;
}
bool lexer::token::is_expression () const
{
	return is_unary_op() ||
		tok == SYNTAX_LPAREN ||
		tok == SYNTAX_LIST_L ||
		tok == SYNTAX_LAMBDA ||
		tok == symbol_token ||
		tok == keyword_false || tok == keyword_true ||
		tok == number_token;
}


std::string lexer::token::two_char::str () const
{
	char buf[3];
	buf[0] = a;
	buf[1] = b;
	buf[2] = '\0';
	return std::string(buf);
}



};
