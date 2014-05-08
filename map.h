#pragma once

#include "value.h"

namespace xy {

class map
{
public:
	typedef uint64_t hash;
	
	
	map (int size = 0);
	map (const std::vector<hash>& keys);
	~map ();
	
	
	bool contains (hash key) const;
	value get (hash key) const;
	bool set (hash key, const value& v);
	
	static hash get_hash (const std::string& key);
	static std::shared_ptr<map> empty ();
	static std::shared_ptr<map> create (const std::vector<hash>& keys,
									const std::vector<value>& vals);
	static std::shared_ptr<map> concat (const std::shared_ptr<map>& a,
							const std::shared_ptr<map>& b);
	
private:
	int index (hash key) const;
	
	
	
	int size;
	hash* keys;
	value* values;
};



};
