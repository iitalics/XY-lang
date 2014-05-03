#pragma once

#include "state.h"

namespace xy {

class lexer;
class environment;
class expression;
class value;
class closure;
struct func_body;
struct function_generator;
struct symbol_locator;


class parser
{
public:
	parser (state& parent, lexer& lex);
	~parser ();
	
	bool parse_env (environment& env);
	
private:
	state& parent;
	lexer& lex;
	std::shared_ptr<symbol_locator> locator;
	
	bool parse_exp (std::shared_ptr<expression>& out);
	bool parse_single_exp (std::shared_ptr<expression>& out);
	bool parse_exp_prologe (std::shared_ptr<expression>& out, std::shared_ptr<expression> in);
	
	bool parse_declare (environment& env, function_generator& g);
	bool parse_function (std::shared_ptr<func_body>& out);
	bool parse_list (std::shared_ptr<expression>& out);
	bool parse_lambda (std::shared_ptr<expression>& out);
	bool parse_list_comp (std::shared_ptr<expression>& out, const std::shared_ptr<expression>& list_exp);
};





};
