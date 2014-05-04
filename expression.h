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
	void push_empty ();
	void add (const std::string& name);
	void pop ();
};



class expression
{
public:
	// inline expression () {}
	
	struct tail_call
	{
		tail_call (function* f);
		
		function* func;
		std::vector<value> args;
		bool do_tail;
	};
	
	virtual ~expression ();
	virtual bool eval (value& out, state::scope& scope);
	virtual bool eval_tail_call (tail_call& tc, value& out, state::scope& scope);
	
	virtual bool locate_symbols (const std::shared_ptr<symbol_locator>& locator);
	virtual bool constant () const;
	
	static std::shared_ptr<expression> create_const (const value& val);
	static std::shared_ptr<expression> create_binary (const std::shared_ptr<expression>& a,
												const std::shared_ptr<expression>& b,
												int op);
	static std::shared_ptr<expression> create_unary (const std::shared_ptr<expression>& a, int op);
	static std::shared_ptr<expression> create_symbol (const std::string& sym);
	static std::shared_ptr<expression> create_closure_ref (int index, int depth = 0);
};


class const_expr;
class binary_expr;
class unary_expr;
class symbol_expr;



class call_expression : public expression
{
public:
	inline call_expression (const std::shared_ptr<expression>& func)
		: func_exp(func)
	{ }
	
	virtual bool eval (value& out, state::scope& scope);
	virtual bool eval_tail_call (tail_call& tc, value& out, state::scope& scope);
	virtual bool locate_symbols (const std::shared_ptr<symbol_locator>& locator);
	
	void add (const std::shared_ptr<expression>& arg);
	
private:
	std::shared_ptr<expression> func_exp;
	std::vector<std::shared_ptr<expression>> args;
};


class list_expression : public expression
{
public:
	list_expression ();
	
	virtual bool eval (value& out, state::scope& scope);
	virtual bool locate_symbols (const std::shared_ptr<symbol_locator>& locator);
	virtual bool constant () const;
	void add (const std::shared_ptr<expression>& arg);
	
private:
	std::vector<std::shared_ptr<expression>> items;
};

class list_comp_expression :
	public expression
{
public:
	list_comp_expression (const std::shared_ptr<expression>& origin, const std::string& it_name);
	
	virtual bool eval (value& out, state::scope& scope);
	virtual bool locate_symbols (const std::shared_ptr<symbol_locator>& locator);
	virtual bool constant () const;
	
	inline void set_filter (const std::shared_ptr<expression>& e) { filter = e; }
	inline void set_map (const std::shared_ptr<expression>& e) { map = e; }
private:
	std::string it_name;
	std::shared_ptr<expression> start, filter, map;
};

class with_expression :
	public expression
{
public:
	virtual bool eval (value& out, state::scope& scope);
	virtual bool eval_tail_call (tail_call& tc, value& out, state::scope& scope);
	virtual bool locate_symbols (const std::shared_ptr<symbol_locator>& locator);
	virtual bool constant () const;
	
	bool add (const std::string& name, const std::shared_ptr<expression>& val);
	inline void set_body (const std::shared_ptr<expression>& e) { body = e; }
	inline bool empty () const { return vars.size() == 0; }
private:
	struct var
	{
		std::string name;
		std::shared_ptr<expression> val;
	};
	
	std::vector<var> vars;
	std::shared_ptr<expression> body;
};


/*
	
*/

};
