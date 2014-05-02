#pragma once

#include "parser.h"

namespace xy {

class param_list;


struct symbol_locator
{
	symbol_locator (environment& e, state& s, lexer& l)
		: env(e), parent(s), lex(l)
	{ }
	
	environment& env;
	state& parent;
	lexer& lex;
	
	std::vector<std::vector<std::string>> symbols;
	
	
	std::ostream& die ();
	bool locate (const std::string& sym, int& out_index, int& out_depth);
	void push_param_list (const param_list& p);
	void pop ();
};



class expression
{
public:
	// inline expression () {}
	
	virtual ~expression ();
	virtual bool eval (value& out, state::scope& scope);
	virtual bool locate_symbols (const std::shared_ptr<symbol_locator>& locator);
	virtual bool constant () const;
	
	static std::shared_ptr<expression> create_const (const value& val);
	static std::shared_ptr<expression> create_binary (const std::shared_ptr<expression>& a,
												const std::shared_ptr<expression>& b,
												int op);
	static std::shared_ptr<expression> create_unary (const std::shared_ptr<expression>& a, int op);
	static std::shared_ptr<expression> create_symbol (const std::string& sym);
	static std::shared_ptr<expression> create_closure_ref (int index, int depth = 0);
	static std::shared_ptr<expression> create_true ();
private:
	static state constant_state;
};



class call_expression : public expression
{
public:
	inline call_expression (const std::shared_ptr<expression>& func)
		: func_exp(func)
	{ }
	
	virtual bool eval (value& out, state::scope& scope);
	virtual bool locate_symbols (const std::shared_ptr<symbol_locator>& locator);
	
	
	void add (const std::shared_ptr<expression>& arg);
	
private:
	std::shared_ptr<expression> func_exp;
	std::vector<std::shared_ptr<expression>> args;
};



};
