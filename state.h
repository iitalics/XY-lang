#pragma once
#include "error.h"

namespace xy {


class state
{
public:
	state ();
	~state ();
	
	bool load (const std::string& filename);
	
	
	inline error_handler& error () { return err_handler; }
	
private:
	bool dead;
	error_handler err_handler;
};





};