#include "include.h"
#include "list.h"
#include "value.h"
#include "state.h"

namespace xy {


#define XY_LIST_DUPLICATE_LENGTH 8

std::shared_ptr<list> list::empty_list(new list());


list::list () {}
list::~list () {}


int list::size () { return 0; }
value list::get (int i) { return value(); }

bool list::equals (const std::shared_ptr<list>& other, state& eval_state)
{
	int s = size();
	int bs = other->size();
	
	if (s != bs)
		return false;
	
	for (int i = 0; i < s; i++)
		if (!get(i).equals(other->get(i), eval_state))
			return false;
	return true;
}
std::shared_ptr<list> list::empty () { return empty_list; }

std::shared_ptr<list> list::concat (const std::shared_ptr<list>& a, const std::shared_ptr<list>& b)
{
	int as = a->size();
	int bs = b->size();
	
	if ((as + bs) <= XY_LIST_DUPLICATE_LENGTH)
	{
		std::vector<value> vs;
		for (int i = 0; i < as; i++)
			vs.push_back(a->get(i));
		for (int i = 0; i < bs; i++)
			vs.push_back(b->get(i));
		return std::shared_ptr<list>(new list_basic(vs));
	}
	else
		return std::shared_ptr<list>(new list_concat(a, b));
}
std::shared_ptr<list> list::sublist (const std::shared_ptr<list>& a, int index)
{
	int size = a->size() - index;
	
	if (size <= XY_LIST_DUPLICATE_LENGTH)
	{
		std::vector<value> vs;
		for (int i = 0; i < size; i++)
			vs.push_back(a->get(index + i));
		return std::shared_ptr<list>(new list_basic(vs));
	}
	else
		return std::shared_ptr<list>(new list_sublist(a, index));
}

/// list_basic


list_basic::list_basic (const std::vector<value>& values)
	: vals(values)
{ }

list_basic::list_basic (const std::initializer_list<value>& values)
{
	for (auto v : values)
		vals.push_back(v);
}

list_basic::~list_basic () {}

int list_basic::size ()
{
	return vals.size();
}
value list_basic::get (int i)
{
	if (i < 0 || i >= size())
		return value();
	else
		return vals[i];
}


/// list_sublist


list_sublist::list_sublist (const std::shared_ptr<list>& other, int s)
	: start(s), end(other->size()), a(other)
{ }
list_sublist::list_sublist (const std::shared_ptr<list>& other, int s, int e)
	: start(s), end(e), a(other)
{
	if (end >= a->size())
		end = a->size();
}

int list_sublist::size ()
{
	return end - start;
}
value list_sublist::get (int i)
{
	return a->get(i + start);
}



/// list_concat


list_concat::list_concat (const std::shared_ptr<list>& a, const std::shared_ptr<list>& b)
	: head(a), tail(b)
{ }

int list_concat::size ()
{
	return head->size() + tail->size();
}
value list_concat::get (int i)
{
	int hs = head->size();
	if (i >= hs)
		return tail->get(i - hs);
	else
		return head->get(i);
}


};
