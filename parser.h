#pragma once

#include "state.h"

namespace xy {

class lexer;
class environment;
class expression;
class value;
class closure;
class soft_function;
struct function_generator;


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
	
	
	bool parse_declare (environment& env, function_generator& g);
};





class expression
{
public:
	// inline expression () {}
	
	virtual ~expression ();
	virtual bool eval (value& out, state::scope& scope);
	virtual bool constant () const;
	
	static std::shared_ptr<expression> create_const (const value& val);
	static std::shared_ptr<expression> create_binary (const std::shared_ptr<expression>& a,
												const std::shared_ptr<expression>& b,
												int op);
	static std::shared_ptr<expression> create_true ();
private:
	static state constant_state;
};






};
