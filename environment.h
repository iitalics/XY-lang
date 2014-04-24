#pragma once

namespace xy {


class state;
class environment
{
public:
	environment (state& parent);
	~environment ();
	
	
private:
	state& parent;
};




};
