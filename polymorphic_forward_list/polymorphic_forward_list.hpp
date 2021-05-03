/* Copyright (C) 2019 Theodoric E. Stier - All Rights Reserved
 * You may use, distribute and modify this code under the
 * terms of the MIT license.
 *
 * INFO: This file is intended to be viewed using a horizontal tab width of 4.
 */

#ifndef POLYMORPHIC_FORWARD_LIST_HPP
#define POLYMORPHIC_FORWARD_LIST_HPP

#include <iterator>
#include <limits>

#if __has_cpp_attribute(nodiscard)
#define PFL_NODISCARD [[nodiscard]]
#else
#define PFL_NODISCARD
#endif

template<class Elem_Base>
class polymorphic_forward_list
{
public:
	using value_type = Elem_Base;
	using size_type = size_t;
	using difference_type = void;
	using reference = value_type &;
	using const_reference = value_type const &;
	using pointer = value_type *;
	using const_pointer = value_type const *;

private:

	//--------------------------------------------------------------------------
	//
	//
	// Node Types
	//
	//
	//--------------------------------------------------------------------------

	struct basic_node;

	struct link
	{
		link() = delete;
		link(link const &) = delete;
		link(link &&) = delete;
		auto operator=(link const &)->link & = delete;
		auto operator=(link &&)->link & = delete;
		~link() = default;

		link(basic_node * next) noexcept :
			next{ next }
		{ }

		basic_node * next;
	};

	struct basic_node : link
	{
		basic_node(link & after, reference ref) noexcept :
			link{ after.next },
			ref{ ref }
		{
			after.next = this;
		}

		virtual ~basic_node() noexcept = default;

		reference ref;
	};

	template<class Elem_Derived>
	struct basic_owner
	{
		template<class ... Args>
		basic_owner(Args && ... args)
			noexcept(noexcept(Elem_Derived{ std::forward<Args>(args) ... })) :
			elem{ std::forward<Args>(args) ... }
		{ }

		Elem_Derived elem;
	};

	template<class Elem_Derived>
	struct node : basic_owner<Elem_Derived>, basic_node
	{
		template<class ... Args>
		node(link & after, Args && ... args)
			noexcept(noexcept(Elem_Derived{ std::forward<Args>(args) ... })) :
			basic_owner<Elem_Derived>{ std::forward<Args>(args) ... },
			basic_node{ after, basic_owner<Elem_Derived>::elem }
		{ }
	};

public:

	//--------------------------------------------------------------------------
	//
	//
	// Iterator Types
	//
	//
	//--------------------------------------------------------------------------

	class iterator;
	class const_iterator;

	class iterator
	{
		friend class polymorphic_forward_list;

	public:
		using value_type = polymorphic_forward_list::value_type;
		using difference_type = polymorphic_forward_list::difference_type;
		using pointer = polymorphic_forward_list::pointer;
		using reference = polymorphic_forward_list::reference;
		using iterator_category = std::forward_iterator_tag;

		iterator() noexcept = default;
		iterator(iterator const &) noexcept = default;
		auto operator=(iterator const &) noexcept->iterator & = default;

		auto operator*() const noexcept -> reference
		{
			return static_cast<basic_node &>(*p).ref;
		}
		auto operator->() const noexcept -> pointer
		{
			return &static_cast<basic_node &>(*p).ref;
		}

		auto operator++() noexcept -> iterator &
		{
			p = p->next;
			return *this;
		}
		auto operator++(int) noexcept -> iterator
		{
			iterator copy = *this;
			p = p->next;
			return copy;
		}

		auto operator!=(iterator const & other) noexcept -> bool
		{
			return p != other.p;
		}
		auto operator==(iterator const & other) noexcept -> bool
		{
			return p == other.p;
		}

	private:
		link * p = nullptr;

		iterator(const_iterator const & other) :
			p{ other.p }
		{ }
		iterator(link * p) noexcept :
			p{ p }
		{ }
	};

	class const_iterator
	{
		friend class polymorphic_forward_list;

	public:
		using value_type = polymorphic_forward_list::value_type;
		using difference_type = polymorphic_forward_list::difference_type;
		using pointer = polymorphic_forward_list::const_pointer;
		using reference = polymorphic_forward_list::const_reference;
		using iterator_category = std::forward_iterator_tag;

		const_iterator(iterator const & other) :
			p{ other.p }
		{ }

		const_iterator() noexcept = default;
		const_iterator(const_iterator const &) noexcept = default;
		auto operator=(const_iterator const &) noexcept
			->const_iterator & = default;

		auto operator*() const noexcept -> reference
		{
			return static_cast<basic_node const &>(*p).ref;
		}
		auto operator->() const noexcept -> pointer
		{
			return &static_cast<basic_node const &>(*p).ref;
		}

		auto operator++() noexcept -> const_iterator &
		{
			p = p->next;
			return *this;
		}
		auto operator++(int) noexcept -> const_iterator
		{
			const_iterator copy = *this;
			p = p->next;
			return copy;
		}

		auto operator!=(const_iterator const & other) noexcept -> bool
		{
			return p != other.p;
		}
		auto operator==(const_iterator const & other) noexcept -> bool
		{
			return p == other.p;
		}

	private:
		link * p = nullptr;

		const_iterator(link * p) noexcept :
			p{ p }
		{ }
	};

	//--------------------------------------------------------------------------
	//
	//
	// Member Functions
	//
	//
	//--------------------------------------------------------------------------

#define PFL_POP(a)														\
	basic_node * const trash = a;										\
	a = trash->next;													\
	delete trash;

#define PFL_SWAP(a, b)													\
	basic_node * const saved = a;										\
	a = b;																\
	b = saved;

	//--------------------------------------------------------------------------
	//
	// Constructors / Assignment Operators
	//
	//--------------------------------------------------------------------------

	polymorphic_forward_list(polymorphic_forward_list const & other) = delete;
	auto operator=(polymorphic_forward_list const & other)
		->polymorphic_forward_list & = delete;

	polymorphic_forward_list() noexcept :
		root{ nullptr }
	{}

	polymorphic_forward_list(polymorphic_forward_list && other) noexcept :
		root{ other.root.next }
	{
		other.root.next = nullptr;
	}

	auto operator=(polymorphic_forward_list && other) noexcept
		-> polymorphic_forward_list &
	{
		auto other_old = other.root.next;
		other.root.next = nullptr;
		auto old = root.next;
		root.next = other_old;
		if (old) delete old;
		return *this;
	}

	~polymorphic_forward_list() noexcept
	{
		while (root.next)
		{
			PFL_POP(root.next);
		}
	}

	//--------------------------------------------------------------------------
	//
	// Assignments
	//
	//--------------------------------------------------------------------------

#define PFL_ASSIGN(op, val)												\
	link assign_root = nullptr;											\
	link * assign_before_end = &assign_root;							\
	try																	\
	{																	\
		op																\
		{																\
			assign_before_end =											\
				new node<Elem_Derived>(*assign_before_end, val);		\
		}																\
	}																	\
	catch (...)															\
	{																	\
		while(assign_root.next)											\
		{																\
			PFL_POP(assign_root.next);									\
		}																\
		throw;															\
	}																	\
	while (root.next)													\
	{																	\
		PFL_POP(root.next);												\
	}																	\
	root.next = assign_root.next;

	template<
		class InputIt,
		class Elem_Derived = typename std::iterator_traits<InputIt>::value_type,
		typename = std::enable_if_t<!std::is_integral_v<InputIt>>>
	polymorphic_forward_list(InputIt first, InputIt last) :
		root{ nullptr }
	{
		link assign_root = nullptr;
		link * assign_before_end = &assign_root;
		try
		{
			while (first != last)
			{
				assign_before_end =
					new node<Elem_Derived>(*assign_before_end, *first++);
			}
		}
		catch (...)
		{
			while (assign_root.next)
			{
				PFL_POP(assign_root.next);
			}
			throw;
		}
		root.next = assign_root.next;
	}

	template<class Elem_Derived>
	void assign(size_type count, Elem_Derived const & value)
	{
		PFL_ASSIGN(for (size_type i = 0; i < count; i++), value);
	}

	template<class InputIt, class Elem_Derived = typename std::iterator_traits<InputIt>::value_type>
	auto assign(InputIt first, InputIt last)
		-> std::enable_if_t<!std::is_integral_v<InputIt>>
	{
		PFL_ASSIGN(while (first != last), *first++);
	}

#undef PFL_ASSIGN

	//--------------------------------------------------------------------------
	//
	// Element Access
	//
	//--------------------------------------------------------------------------

	PFL_NODISCARD auto front() noexcept -> reference
	{
		return root.next->ref;
	}
	PFL_NODISCARD auto front() const noexcept -> const_reference
	{
		return root.next->ref;
	}

	//--------------------------------------------------------------------------
	//
	// Iterators
	//
	//--------------------------------------------------------------------------

	PFL_NODISCARD auto before_begin() noexcept -> iterator
	{
		return &root;
	}
	PFL_NODISCARD auto begin() noexcept -> iterator
	{
		return root.next;
	}
	PFL_NODISCARD auto end() noexcept -> iterator
	{
		return nullptr;
	}

	PFL_NODISCARD auto before_begin() const noexcept -> const_iterator
	{
		return const_cast<link *>(&root);
	}
	PFL_NODISCARD auto begin() const noexcept -> const_iterator
	{
		return root.next;
	}
	PFL_NODISCARD auto end() const noexcept -> const_iterator
	{
		return nullptr;
	}

	PFL_NODISCARD auto cbefore_begin() const noexcept -> const_iterator
	{
		return const_cast<link *>(&root);
	}
	PFL_NODISCARD auto cbegin() const noexcept -> const_iterator
	{
		return root.next;
	}
	PFL_NODISCARD auto cend() const noexcept -> const_iterator
	{
		return nullptr;
	}

	//--------------------------------------------------------------------------
	//
	// Capacity
	//
	//--------------------------------------------------------------------------

	PFL_NODISCARD auto empty() const noexcept -> bool
	{
		return !root.next;
	}

	PFL_NODISCARD auto max_size() const noexcept -> size_type
	{
		return std::numeric_limits<size_type>::max();
	}

	//--------------------------------------------------------------------------
	//
	// Modifiers
	//
	//--------------------------------------------------------------------------

	void clear() noexcept
	{
		while (root.next)
		{
			PFL_POP(root.next);
		}
	}

	//--------------------------------------------------------------------------
	// Insertions
	//--------------------------------------------------------------------------

#define PFL_INSERT(op, val)												\
	link insert_root = nullptr;											\
	link * insert_before_end = &insert_root;							\
	try																	\
	{																	\
		op																\
		{																\
			insert_before_end											\
				= new node<Elem_Derived>(*insert_before_end, val);		\
		}																\
	}																	\
	catch (...)															\
	{																	\
		while(insert_root.next)											\
		{																\
			PFL_POP(insert_root.next);									\
		}																\
		throw;															\
	}																	\
	PFL_SWAP(pos.p->next, insert_root.next);							\
	return insert_before_end;

	template<class Elem_Derived>
	auto insert_after(const_iterator pos, Elem_Derived const & value)
		-> iterator
	{
		return new node<Elem_Derived>(*pos.p, value);
	}

	template<class Elem_Derived>
	auto insert_after(const_iterator pos, Elem_Derived && value) -> iterator
	{
		return new node<Elem_Derived>(*pos.p, std::move(value));
	}

	template<class Elem_Derived>
	auto insert_after(
		const_iterator pos,
		size_type count,
		Elem_Derived const & value) -> iterator
	{
		PFL_INSERT(for (size_type i = 0; i < count; i++), value);
	}

	template<
		class InputIt,
		class Elem_Derived = typename std::iterator_traits<InputIt>::value_type>
	auto insert_after(const_iterator pos, InputIt first, InputIt last)
		-> std::enable_if_t<!std::is_integral_v<InputIt>, iterator>
	{
		PFL_INSERT(while (first != last), *first++);
	}

#undef PFL_INSERT

	//--------------------------------------------------------------------------
	// Erasures
	//--------------------------------------------------------------------------

	template<class Elem_Derived = Elem_Base, class ... Args>
	auto emplace_after(const_iterator pos, Args && ... args) -> iterator
	{
		return new node<Elem_Derived>(*pos.p, std::forward<Args>(args) ...);
	}

	auto erase_after(const_iterator pos) noexcept
	{
		PFL_POP(pos.p->next)
			return pos.p->next;
	}

	auto erase_after(const_iterator first, const_iterator last) noexcept
		-> iterator
	{
		while ((first.p->next) != last.p)
		{
			PFL_POP(first.p->next);
		}
		return last.p;
	}

	//--------------------------------------------------------------------------
	// Push / Pop / Emplace / Swap
	//--------------------------------------------------------------------------

	template<class Elem_Derived>
	void push_front(Elem_Derived const & value)
	{
		new node<Elem_Derived>(root, value);
	}

	template<class Elem_Derived>
	void push_front(Elem_Derived && value)
	{
		new node<Elem_Derived>(root, std::move(value));
	}

	template<class Elem_Derived = Elem_Base, class ... Args>
	auto emplace_front(Args && ... args) -> Elem_Derived &
	{
		basic_node * const new_node =
			new node<Elem_Derived>(root, std::forward<Args>(args) ...);
		return static_cast<Elem_Derived &>(new_node->ref);
	}

	void pop_front()
	{
		PFL_POP(root.next);
	}

	void swap(polymorphic_forward_list & other) noexcept
	{
		PFL_SWAP(root.next, other.root.next);
	}

	//--------------------------------------------------------------------------
	//
	// Operations
	//
	//--------------------------------------------------------------------------

#define PFL_SPLICE_ONE(a, b)											\
	basic_node * const saved = a;										\
	a = b;																\
	b = a->next;														\
	a->next = saved;

	//--------------------------------------------------------------------------
	// Merges
	//--------------------------------------------------------------------------

#define PFL_MERGE(op)													\
	if (this == &other) return;											\
	if (!other.root.next) return;										\
	link * pivot = &root;												\
	for (; pivot->next && other.root.next; pivot = pivot->next)			\
	{																	\
		if (op)															\
		{																\
			PFL_SPLICE_ONE(pivot->next, other.root.next);				\
		}																\
	}																	\
	if (other.root.next)												\
	{																	\
		PFL_SWAP(other.root.next, pivot->next);							\
	}

	void merge(polymorphic_forward_list & other)
		noexcept(noexcept(other.root.next->ref < root.next->ref))
	{
		PFL_MERGE(other.root.next->ref < pivot->next->ref);
	}

	void merge(polymorphic_forward_list && other)
		noexcept(noexcept(other.root.next->ref < root.next->ref))
	{
		PFL_MERGE(other.root.next->ref < pivot->next->ref);
	}

	template<class Compare>
	void merge(polymorphic_forward_list & other, Compare comp)
		noexcept(noexcept(comp(other.root.next->ref, this->root.next->ref)))
	{
		PFL_MERGE(comp(other.root.next->ref, pivot->next->ref));
	}

	template<class Compare>
	void merge(polymorphic_forward_list && other, Compare comp)
		noexcept(noexcept(comp(other.root.next->ref, this->root.next->ref)))
	{
		PFL_MERGE(comp(other.root.next->ref, pivot->next->ref));
	}

#undef PFL_MERGE

	//--------------------------------------------------------------------------
	// Splices
	//--------------------------------------------------------------------------

#define PFL_SPLICE(a, b, c, condition)									\
	basic_node * const saved = a->next;									\
	a->next = b;														\
	b = c;																\
	while (condition) a = a->next;										\
	a->next = saved;

	void splice_after(const_iterator pos, polymorphic_forward_list & other)
		noexcept
	{
		PFL_SPLICE(pos.p, other.root.next, nullptr, pos.p->next);
	}

	void splice_after(const_iterator pos, polymorphic_forward_list && other)
		noexcept
	{
		PFL_SPLICE(pos.p, other.root.next, nullptr, pos.p->next);
	}

	void splice_after(const_iterator pos, const_iterator it) noexcept
	{
		PFL_SPLICE_ONE(pos.p->next, it.p->next);
	}

	void splice_after(
		const_iterator pos,
		const_iterator first,
		const_iterator last)
		noexcept
	{
		PFL_SPLICE(pos.p, first.p->next, last.p, pos.p->next != last.p);
	}

#undef PFL_SPLICE

	//--------------------------------------------------------------------------
	// Removals
	//--------------------------------------------------------------------------

#define PFL_REMOVE(op)													\
	size_type removed_count = 0;										\
	for (link * pivot = &root; pivot->next;)							\
	{																	\
		if (op)															\
		{																\
			PFL_POP(pivot->next);										\
			removed_count++;											\
		}																\
		else															\
		{																\
			pivot = pivot->next;										\
		}																\
	}																	\
	return removed_count;

	auto remove(const_reference value) -> size_type
	{
		PFL_REMOVE(pivot->next->ref == value);
	}

	template<class UnaryPredicate>
	auto remove_if(UnaryPredicate p) -> size_type
	{
		PFL_REMOVE(p(pivot->next->ref));
	}

	void reverse() noexcept
	{
		link reverse_root = nullptr;
		while (root.next)
		{
			PFL_SPLICE_ONE(reverse_root.next, root.next);
		}
		root.next = reverse_root.next;
	}

#undef PFL_REMOVE

#undef PFL_SPLICE_ONE

#undef PFL_POP
#undef PFL_SWAP

private:
	link root;
};

//------------------------------------------------------------------------------
//
//
// Non-member Functions
//
//
//------------------------------------------------------------------------------

template<class T>
PFL_NODISCARD auto operator==(
	polymorphic_forward_list<T> const & lhs,
	polymorphic_forward_list<T> const & rhs)
	noexcept(noexcept(*lhs.begin() == *rhs.begin()))
	-> bool
{
	auto left = lhs.begin();
	auto right = rhs.begin();
	auto const end = lhs.end();
	for (;;)
	{
		if (left == end)
		{
			return right == end;
		}
		if (right == end)
		{
			return false;
		}
		if (!(*left++ == *right++)) return false;
	}
}
template<class T>
PFL_NODISCARD auto operator!=(
	polymorphic_forward_list<T> const & lhs,
	polymorphic_forward_list<T> const & rhs)
	noexcept(noexcept(*lhs.begin() == *rhs.begin()))
	-> bool
{
	auto left = lhs.begin();
	auto right = rhs.begin();
	auto const end = lhs.end();
	for (;;)
	{
		if (left == end)
		{
			return right != end;
		}
		if (right == end)
		{
			return true;
		}
		if (!(*left++ == *right++)) return true;
	}
}
template<class T>
PFL_NODISCARD auto operator<(
	polymorphic_forward_list<T> const & lhs,
	polymorphic_forward_list<T> const & rhs)
	noexcept(
		noexcept(*lhs.begin() < *rhs.begin()) &&
		noexcept(*rhs.begin() < *lhs.begin()))
	-> bool
{
	auto left = lhs.begin();
	auto right = rhs.begin();
	auto const end = lhs.end();
	for (;;)
	{
		if (left == end)
		{
			return right != end;
		}
		if (right == end)
		{
			return false;
		}
		if (*left < *right) return true;
		if (*right++ < *left++) return false;
	}
}
template<class T>
PFL_NODISCARD auto operator>=(
	polymorphic_forward_list<T> const & lhs,
	polymorphic_forward_list<T> const & rhs)
	noexcept(
		noexcept(*lhs.begin() < *rhs.begin()) &&
		noexcept(*rhs.begin() < *lhs.begin()))
	-> bool
{
	auto left = lhs.begin();
	auto right = rhs.begin();
	auto const end = lhs.end();
	for (;;)
	{
		if (left == end)
		{
			return right == end;
		}
		if (right == end)
		{
			return true;
		}
		if (*left < *right) return false;
		if (*right++ < *left++) return true;
	}
}
template<class T>
PFL_NODISCARD auto operator>(
	polymorphic_forward_list<T> const & lhs,
	polymorphic_forward_list<T> const & rhs)
	noexcept(
		noexcept(*lhs.begin() < *rhs.begin()) &&
		noexcept(*rhs.begin() < *lhs.begin()))
	-> bool
{
	auto left = lhs.begin();
	auto right = rhs.begin();
	auto const end = lhs.end();
	for (;;)
	{
		if (right == end)
		{
			return left != end;
		}
		if (left == end)
		{
			return false;
		}
		if (*right < *left) return true;
		if (*left++ < *right++) return false;
	}
}
template<class T>
PFL_NODISCARD auto operator<=(
	polymorphic_forward_list<T> const & lhs,
	polymorphic_forward_list<T> const & rhs)
	noexcept(
		noexcept(*lhs.begin() < *rhs.begin()) &&
		noexcept(*rhs.begin() < *lhs.begin()))
	-> bool
{
	auto left = lhs.begin();
	auto right = rhs.begin();
	auto const end = lhs.end();
	for (;;)
	{
		if (right == end)
		{
			return left == end;
		}
		if (left == end)
		{
			return true;
		}
		if (*right < *left) return false;
		if (*left++ < *right++) return true;
	}
}

#undef PFL_NODISCARD

#endif
