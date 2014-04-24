#include "include.h"
#include "parser.h"
#include "lexer.h"
#include "state.h"
#include "value.h"

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









bool parser::parse_single_exp (std::shared_ptr<expression>& out)
{
	switch (lex.current().tok)
	{
	case lexer::token::number_token:
		out = expression::create_number(lex.current().num);
		break;
		
	case '(':
		if (!lex.advance())
			return false;
		if (!parse_exp(out))
			return false;
		if (!lex.expect(')'))
			return false;
		break;
	
	default:
		return lex.unexpect();
	}
	
	return lex.advance();
}

bool parser::parse_exp (std::shared_ptr<expression>& out)
{
	return lex.unexpect();
}







expression::~expression () { }
bool expression::eval (value& out, state& s, std::shared_ptr<closure> closure)
{
	s.error().die() << "Unimplemented expression?";
	return false;
}


class number_exp : public expression
{
public:
	number_exp (number n)
		: num(n) {}
	virtual bool eval (value& out, state& s, std::shared_ptr<closure> closure)
	{
		out = value::from_number(num);
		return true;
	}
	number num;
};
class true_exp : public expression
{
public:
	virtual bool eval (value& out, state& s, std::shared_ptr<closure> closure)
	{
		out = value::from_bool(true);
		return true;
	}
};
class false_exp : public expression
{
public:
	virtual bool eval (value& out, state& s, std::shared_ptr<closure> closure)
	{
		out = value::from_bool(false);
		return true;
	}
};
std::shared_ptr<expression> expression::create_number (number num)
{
	return std::shared_ptr<expression>(new number_exp(num));
}
std::shared_ptr<expression> expression::create_bool (bool b)
{
	if (b)
		return std::shared_ptr<expression>(new true_exp());
	else
		return std::shared_ptr<expression>(new false_exp());
}


};
