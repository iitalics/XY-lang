#pragma once
#include "error.h"
#include "environment.h"

namespace xy {


class state
{
public:
	state ();
	~state ();
	
	bool load (const std::string& filename);
	
	
	inline error_handler& error () { return err_handler; }
	inline environment& global () { return global_env; }
	
private:
	error_handler err_handler;
	environment global_env;
};





};