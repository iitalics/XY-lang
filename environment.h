#pragma once

namespace xy {


class state;
class function;
class native_function;
class soft_function;

class environment
{
public:
	environment (state& parent);
	~environment ();
	
	std::shared_ptr<function> find_function (const std::string& name);
	void add_function (const std::shared_ptr<function>& func);
	
	std::shared_ptr<soft_function> find_or_add (const std::string& name);
	
	template <typename T>
	void add_native (const std::string& name, const T& func)
	{
		add_function(std::shared_ptr<function>(
			new native_function(name, func)));
	}
	
private:
	state& parent;
	std::vector<std::shared_ptr<function>> funcs;
};




};
