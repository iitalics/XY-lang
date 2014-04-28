#pragma once
#include "error.h"
#include "environment.h"

namespace xy {

// TODO: implement closures

class closure;
class value;


class state
{
public:
	state ();
	~state ();
	
	bool load (const std::string& filename);
	
	
	inline error_handler& error () { return err_handler; }
	inline environment& global () { return global_env; }
	
	// meant to be a VERY simplistic class, why everything is inlined
	struct scope
	{
		inline scope (state& p, std::shared_ptr<closure> c)
			: parent(p), local(c) {}
		inline scope (state& p)
			: parent(p) {}
		inline state& operator() () { return parent; }
		
		state& parent;
		std::shared_ptr<closure> local;
	};
	
	
	
private:
	error_handler err_handler;
	environment global_env;
};




class argument_list;
class closure
{
public:
	closure (int size, const std::shared_ptr<closure>& parent =
						std::shared_ptr<closure>(nullptr));
	closure (const argument_list& args, const std::shared_ptr<closure>& parent =
						std::shared_ptr<closure>(nullptr));
	~closure ();
	
	value get (int index, int depth = 0);
	bool set (int index, const value& val);	// muh stateless programming language
	
	int size () const;
private:
	std::shared_ptr<closure> parent;
	int closure_size;
	value* values;
};


};