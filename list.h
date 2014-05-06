#pragma once


#include "state.h"
#include <initializer_list>

namespace xy {

class list
{
public:
	list ();
	virtual ~list ();
	
	// THESE MUST RETURN A VALUE meh this bad
	// out of bounds == nil or void?
	virtual int size ();
	virtual value get (int i);
	
	bool equals (const std::shared_ptr<list>& other, state& eval_state);
	
	static std::shared_ptr<list> empty ();
	static std::shared_ptr<list> concat (const std::shared_ptr<list>& a, const std::shared_ptr<list>& b);
	static std::shared_ptr<list> sublist (const std::shared_ptr<list>& a, int index);
	static std::shared_ptr<list> basic (const std::vector<value>& values);
private:
	static std::shared_ptr<list> empty_list;
	
protected:
	bool is_sublist;
};


class list_basic
	: public list
{
public:
	list_basic (const std::vector<value>& values);
	list_basic (const std::initializer_list<value>& values);
	
	virtual ~list_basic ();
	virtual int size ();
	virtual value get (int i);
private:
	std::vector<value> vals;
};


class list_sublist
	: public list
{
public:
	list_sublist (const std::shared_ptr<list>& other, int start);
	list_sublist (const std::shared_ptr<list>& other, int start, int end);
	
	virtual int size ();
	virtual value get (int i);
	
	int start, end;
	std::shared_ptr<list> a;
};



class list_concat
	: public list
{
public:
	list_concat (const std::shared_ptr<list>& a, const std::shared_ptr<list>& b);
	
	virtual int size ();
	virtual value get (int i);
private:
	std::shared_ptr<list> head, tail;
};



};
