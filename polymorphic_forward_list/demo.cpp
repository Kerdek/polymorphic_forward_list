#pragma once

#include "polymorphic_forward_list.h"

#include <iostream>
#include <string>

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
	polymorphic_forward_list<one> pl;
	
	// emplace_front defaults to the basic element type
	pl.emplace_front("a");

	// emplace_front can be used to create nodes of derived types
	auto bar_it = pl.emplace_after<two>(pl.begin(), "b", "c");
	pl.emplace_after<three>(bar_it, "d", "e", "f");
	
	for (auto & elem : pl)
	{
		std::cout << elem.get() << '\n';
	}
	
	// elements need not have virtual destructors, they are destroyed through the derived type
}