#pragma once

#include "polymorphic_forward_list.h"

#include <iostream>
#include <string>
#include <forward_list>

int main()
{
	polymorphic_forward_list<int> l1;
	polymorphic_forward_list<int> l2;
	
	l1.emplace_front(2);
	l1.emplace_front(1);

	l2.emplace_front(4);
	l2.emplace_front(3);

	l1.splice_after(l1.begin(), l2.before_begin());

	for (auto & elem : l1)
	{
		std::cout << elem;
	}
	
	std::cout << '\n';
	
	for (auto & elem : l2)
	{
		std::cout << elem;
	}
}