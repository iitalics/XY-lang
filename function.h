#pragma once
#include "parser.h"
#include "state.h"

#include <initializer_list>

namespace xy {

class value;
class param_list;

struct argument_list
{
	argument_list (int size = 0);
	argument_list (const param_list& params);
	argument_list (const argument_list& other);
	argument_list (std::initializer_list<value> values);
	~argument_list ();
	
	int size;
	value* values;
};


class function
{
public:
	function (const std::string& name, bool native = true);
	virtual ~function ();
	
	inline bool is_native () const { return native; }
	inline std::string name () const { return func_name; }
	inline bool is_lambda () const { return func_name.size() == 0; }
	
	virtual bool call (value& out, const argument_list& args, state::scope& scope);
protected:
	std::string func_name;
	bool native;
};


class param_list
{
public:
	//param_list ();
	//~param_list ();
	
	// let (var : cond) = ...
	void add_param (const std::string& name,
					std::shared_ptr<expression> condition);
	// let (var) = ...
	void add_param (const std::string& name);
	// let (value) = ...
	void add_param (std::shared_ptr<expression> a);
	
	std::shared_ptr<expression> condition (int index);
	std::shared_ptr<expression> condition (const std::string& name);
	
	int size () const;
	int locate (const std::string& name) const; // locate(n) = [-1] OR [0, size )
	std::string param_name (int index) const;
	
	
	bool satisfies (bool& out, state::scope& scope);
private:
	struct param
	{
		inline param (const std::string& n, const std::shared_ptr<expression>& e)
			: name(n), cond(e) {}
		inline param (const std::shared_ptr<expression>& e)
			: name(""), cond(e) {}
		inline param ()
			: name("") {}
		
		
		std::string name;
		std::shared_ptr<expression> cond;
	};
	std::vector<param> params;
};




struct func_body
{
	param_list params;
	std::shared_ptr<expression> body;
};


class soft_function : public function
{
public:
	// normal 'named' function
	soft_function (const std::string& name);
	// lambda
	soft_function (const std::shared_ptr<closure>& scope);
	
	virtual ~soft_function ();
	
	void add_overload (const std::shared_ptr<func_body>& o);
	
	virtual bool call (value& out, const argument_list& args, state::scope& scope);
private:
	std::vector<std::shared_ptr<func_body>> overloads;
	std::shared_ptr<closure> parent_closure;
};


};
