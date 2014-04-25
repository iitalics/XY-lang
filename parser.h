#pragma once

#include "state.h"

namespace xy {

class lexer;
class environment;
class expression;
class value;


// TODO: implement closures
class closure
{
public: inline closure () {}
};

class parser
{
public:
	parser (state& parent, lexer& lex);
	~parser ();
	
	bool parse_env (environment& env);
	
private:
	state& parent;
	lexer& lex;
	
	bool parse_single_exp (std::shared_ptr<expression>& out);
	
	// does entire mathematical expression
	bool parse_exp (std::shared_ptr<expression>& out);
	
};





class expression
{
public:
	// inline expression () {}
	
	virtual ~expression ();
	virtual bool eval (value& out, state& s, std::shared_ptr<closure> closure
													= std::shared_ptr<closure>(nullptr));
	virtual bool constant () const;
	
	static std::shared_ptr<expression> create_const (const value& val);
	static std::shared_ptr<expression> create_number (number num);
	static std::shared_ptr<expression> create_bool (bool b);
	static std::shared_ptr<expression> create_binary (const std::shared_ptr<expression>& a,
												const std::shared_ptr<expression>& b,
												int op);
private:
	static state constant_state;
};



};
