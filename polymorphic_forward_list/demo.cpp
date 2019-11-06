#pragma once

#include "polymorphic_forward_list.h"

#include <iostream>
#include <string>
#include <forward_list>

class one
{
	std::string first;
public:
	one(std::string first) :
		first{ std::move(first) }
	{ }
	
	~one()
	{
		std::cout << "~one()\n";
	}

	virtual std::string get() const
	{
		return first;
	}
};

class two : public one
{
	std::string last;
public:
	two(std::string first, std::string last) :
		one{ std::move(first) },
		last{ std::move(last) }
	{ }

	~two()
	{
		std::cout << "~two()";
	}

	std::string get() const override
	{
		return one::get() + ' ' + last;
	}
};

class three : public two
{
	std::string third;
public:
	three(std::string first, std::string last, std::string third) :
		two{ std::move(first), std::move(last) },
		third{ std::move(third) }
	{ }

	~three()
	{
		std::cout << "~three()";
	}

	std::string get() const override
	{
		return two::get() + ' ' + third;
	}
};

int main()
{
	std::forward_list<int> wut;
	polymorphic_forward_list<one> pl;
	
	// emplace_front defaults to the basic element type
	pl.emplace_front("a");

	// emplace_front can be used to create nodes of derived types
	auto bar_it = pl.emplace_after<two>(pl.begin(), "b", "c");
	pl.emplace_after<three>(bar_it, "d", "e", "f");

	polymorphic_forward_list<int> pl2;

	pl2.emplace_front(4);
	pl2.emplace_front(2);
	pl2.emplace_front(1);

	polymorphic_forward_list<int> pl3;

	pl3.emplace_front(4);
	pl3.emplace_front(3);
	pl3.emplace_front(1);

	pl3.emplace_front(0);

	pl2.merge(pl3);
	//pl2.splice_after(pl2.begin(), pl3);
	
	for (auto & elem : pl2)
	{
		std::cout << elem << '\n';
	}
	
	// elements need not have virtual destructors, they are destroyed through the derived type
}