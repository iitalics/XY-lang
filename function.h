#pragma once

namespace xy {



class function
{
public:
	function (bool native);
	virtual ~function ();
	
	inline bool is_native () const { return native; }
	
private:
	bool native;
};



};
