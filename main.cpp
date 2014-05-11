#include "include.h"
#include "state.h"

#include "function.h"
#include "value.h"
#include "list.h"


#define XY_VERSION "version 0.9.2 beta (c++11 build)"
#define XY_COMPILE_INFO "last modified: may 11, 2014"




static int help_text ()
{
	std::cout << "usage:  xy [flags] PROGRAM [program arguments]\n"
	             "\n"
				 "   --version          display version info\n"
				 "   -h, --help         show this help text\n";
	return 0;
}

static int version_info ()
{
	std::cout << "xy standalone interpreter\n"
				 "| " XY_VERSION "\n"
				 "| " XY_COMPILE_INFO "\n";
	return 0;
}


int main (int argc, char** argv)
{
	if (argc <= 1)
		return help_text();
	
	
	xy::state xy;
	
	int start;
	
	for (start = 1; start <= argc; start++)
	{
		std::string arg(argv[start]);
		
		if (arg == "--version")
			return version_info();
		else if (arg == "-h" || arg == "--help")
			return help_text();
		else
			break;
	}
	
	if (!xy.load(std::string(argv[start])))
		goto fail;
	
	{
		auto main_func = xy.global().find_function("main");
		if (main_func != nullptr)
		{
			xy::state::scope scope(xy);
			xy::value output;
			
			std::vector<xy::value> arg_strings;
			for (int i = start + 1; i < argc; i++)
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