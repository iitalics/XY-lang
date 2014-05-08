#include "include.h"
#include "map.h"

namespace xy {




map::map (int len)
	: size(len),
	  keys(size == 0 ? nullptr : new hash[len]),
	  values(size == 0 ? nullptr : new value[len])
{
	hash empty = get_hash("");
	
	for (int i = len; i--;)
		keys[i] = empty;
}
map::map (const std::vector<hash>& ka)
	: size(ka.size()),
	  keys(size == 0 ? nullptr : new hash[size]),
	  values(size == 0 ? nullptr : new value[size])
{
	int i = 0;
	for (const hash& k : ka)
		keys[i++] = k;
}
map::~map ()
{
	delete[] keys;
	delete[] values;
}

map::hash map::get_hash (const std::string& key)
{
	// http://en.wikipedia.org/wiki/Fowler_Noll_Vo_hash
	
	const hash constexpr fnv_offset = 14695981039346656037;
	const hash constexpr fnv_prime = 1099511628211;
	
	/*
	
	let get_hash (str) = 
		foldl(@(z, c) = xor(z * fnv_prime(), c -> int),
		      fnv_offset(), str)
	
	*/
	
	hash k = fnv_offset;
	for (auto c : key)
		k = (k * fnv_prime) ^ c;
	return k;
}

bool map::contains (hash key) const
{
	return index(key) >= 0;
}
value map::get (hash key) const
{
	int i = index(key);
	if (i >= 0)
		return values[i];
	else
		return value();
}
bool map::set (hash key, const value& v)
{
	int i = index(key);
	if (i >= 0)
	{
		values[i] = v;
		return true;
	}
	else
		return false;
}
int map::index (hash key) const
{
	for (int i = 0; i < size; i++)
		if (keys[i] == key)
			return i;
	return -1;
}




std::shared_ptr<map> map::empty ()
{
	return std::shared_ptr<map>(new map(0));
}
std::shared_ptr<map> map::create (const std::vector<hash>& keys,
								const std::vector<value>& vals)
{
	std::shared_ptr<map> m(new map(keys));
	int i = 0;
	for (const value& v : vals)
		m->values[i++] = v;
	return m;
}
std::shared_ptr<map> map::concat (const std::shared_ptr<map>& a,
									const std::shared_ptr<map>& b)
{
	std::vector<hash> keys;
	std::vector<value> values;
	
	for (int i = 0, size = a->size; i < size; i++)
	{
		keys.push_back(a->keys[i]);
		values.push_back(a->values[i]);
	}
	
	for (int i = 0, size = b->size; i < size; i++)
	{
		int j = a->index(b->keys[i]);
		if (j >= 0)
			values[j] = b->values[i];
		else
		{
			keys.push_back(b->keys[i]);
			values.push_back(b->values[i]);
		}
	}
	
	return create(keys, values);
}


};
