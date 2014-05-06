#include "include.h"
#include "state.h"

#include "function.h"
#include "value.h"
#include "list.h"

int main (int argc, char** argv)
{
	if (argc <= 1)
	{
		std::cerr << "Usage: xy FILE1, FILE2..." << std::endl;
		return 0;
	}
	
	
	xy::state xy;
	
	if (!xy.load(std::string(argv[1])))
		goto fail;
	
	{
		auto main_func = xy.global().find_function("main");
		if (main_func != nullptr)
		{
			xy::state::scope scope(xy);
			xy::value output;
			
			std::vector<xy::value> arg_strings;
			for (int i = 2; i < argc; i++)
				arg_strings.push_back(xy::value::from_string(std::string(argv[i])));
			
			xy::argument_list args
				{
					xy::value::from_list(xy::list::basic(arg_strings))
				};
			
			if (!main_func->call(output, args, scope))
				goto fail;
		}
		else
			std::cout << "no main function found" << std::endl;
	}
	
	return 0;
	
fail:
	xy.error().dump();
	return -1;
}