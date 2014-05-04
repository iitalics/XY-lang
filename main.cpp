#include "include.h"
#include "state.h"

#include "function.h"
#include "value.h"

int main (int argc, char** argv)
{
	xy::state xy;
	
	if (argc <= 1)
	{
		std::cerr << "Usage: xy FILE1, FILE2..." << std::endl;
		return 0;
	}
	
	for (int i = 1; i < argc; i++)
		if (!xy.load(std::string(argv[i])))
			goto fail;
	
	{
		auto main_func = xy.global().find_function("main");
		if (main_func != nullptr)
		{
			xy::state::scope scope(xy);
			xy::value output;
			
			xy::argument_list args
				{
				};
			
			if (!main_func->call(output, args, scope))
				goto fail;
			else
			{
				std::cout << "main =\n   " << output.to_str() << std::endl;
			}
		}
		else
			std::cout << "no main function found" << std::endl;
	}
	
	return 0;
	
fail:
	xy.error().dump();
	return -1;
}