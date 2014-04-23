#include "include.h"
#include "state.h"

int main ()
{
	xy::state xy;
	
	
	if (!xy.load("test.xy"))
		xy.error().dump();
	
	
	return 0;
}