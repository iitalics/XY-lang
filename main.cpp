#include "include.h"
#include "state.h"

#include "function.h"
#include "value.h"

int main ()
{
	xy::state xy;
	
	
	if (!xy.load("test.xy"))
		xy.error().dump();
	else
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
				xy.error().dump();
			else
			{
				std::cout << "main =\n   " << output.to_str() << std::endl;
			}
		}
		else
			std::cout << "no main function found" << std::endl;
	}
	
	return 0;
}