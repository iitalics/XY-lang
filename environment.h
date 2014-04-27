#pragma once

namespace xy {


class state;
class function;

class environment
{
public:
	environment (state& parent);
	~environment ();
	
	std::shared_ptr<function> find_function (const std::string& name);
	void add_function (const std::shared_ptr<function>& func);
	
private:
	state& parent;
	std::vector<std::shared_ptr<function>> funcs;
};




};
