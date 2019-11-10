#ifndef POLYMORPHIC_FORWARD_LIST_HPP
#define POLYMORPHIC_FORWARD_LIST_HPP

#include <iterator>	

#define PFL_ERASE(a)													\
	basic_node * trash = a;												\
	a = trash->next;													\
	delete trash;

#define PFL_SPLICE(a, b)												\
	basic_node * saved = a;												\
	a = b;																\
	b = a->next;														\
	a->next = saved;

#define PFL_SPLICE_WHILE(a, b, c, condition)							\
	basic_node * saved = a->next;										\
	a->next = b;														\
	b = c;																\
	while (condition) a = a->next;										\
	a->next = saved;

#define PFL_SWAP(a, b)													\
	basic_node * saved = a;												\
	a = b;																\
	b = saved;

#define PFL_MERGE(op)													\
	if (this == &other) return;											\
	if (!other.root.next) return;										\
	link * pivot = &root;												\
	for (; pivot->next && other.root.next; pivot = pivot->next)			\
	{																	\
		if (op)															\
		{																\
			PFL_SPLICE(pivot->next, other.root.next);					\
		}																\
	}																	\
	if (other.root.next)												\
	{																	\
		PFL_SWAP(other.root.next, pivot->next);							\
	}

#define PFL_FOR_ALL(at, op)												\
	while(at)															\
	{																	\
		op;																\
	}

#define PFL_ASSIGN(op, val)												\
	using node_elem_type =												\
		std::remove_const_t<std::remove_reference_t<decltype(val)>>;	\
	link assign_root = nullptr;											\
	link * assign_before_end = &assign_root;							\
	try																	\
	{																	\
		op																\
		{																\
			assign_before_end =											\
				new node<node_elem_type>(*assign_before_end, val);		\
		}																\
	}																	\
	catch (...)															\
	{																	\
		PFL_FOR_ALL(assign_root.next, PFL_ERASE(assign_root.next))		\
		throw;															\
	}																	\
	while (root.next)													\
	{																	\
		PFL_ERASE(root.next);											\
	}																	\
	root.next = assign_root.next;

#define PFL_INSERT(op, val)												\
	using node_elem_type =												\
		std::remove_const_t<std::remove_reference_t<decltype(val)>>;	\
	link insert_root;													\
	link * insert_before_end = &insert_root;							\
	try																	\
	{																	\
		op																\
		{																\
			insert_before_end											\
				= new node<node_elem_type>(insert_before_end, val);		\
		}																\
	}																	\
	catch (...)															\
	{																	\
		PFL_FOR_ALL(insert_root.next, PFL_ERASE(insert_root.next))		\
		throw;															\
	}																	\
	PFL_SWAP(pos.p->next, insert_root.next);							\
	return insert_before_end;

#define PFL_REMOVE(op)													\
	size_type removed_count = 0;										\
	for (link * pivot = &root; pivot->next; pivot = pivot->next)		\
	{																	\
		if (op)															\
		{																\
			PFL_ERASE(pivot->next);										\
			removed_count++;											\
		}																\
	}																	\
	return removed_count;

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
	struct basic_node;

	struct link
	{
		link() = delete;
		link(link const &) = delete;
		link(link &&) = delete;
		link & operator=(link const &) = delete;
		link & operator=(link &&) = delete;
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

	class iterator;
	class const_iterator;

	class iterator
	{
		friend class polymorphic_forward_list;

	public:
		using value_type = value_type;
		using difference_type = difference_type;
		using pointer = pointer;
		using reference = reference;
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
		using difference_type = void;
		using value_type = value_type;
		using pointer = const_pointer;
		using reference = const_reference;
		using iterator_category = std::forward_iterator_tag;

		const_iterator(iterator const & other) :
			p{ other.p }
		{ }

		const_iterator() noexcept = default;
		const_iterator(const_iterator const &) noexcept = default;
		auto operator=(const_iterator const &) noexcept
			-> const_iterator & = default;

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
	
	polymorphic_forward_list(polymorphic_forward_list const & other) = delete;
	auto operator=(polymorphic_forward_list const & other)
		-> polymorphic_forward_list & = delete;

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
		PFL_SWAP(root.next, other.root.next);
	}

	~polymorphic_forward_list() noexcept
	{
		PFL_FOR_ALL(root.next, PFL_ERASE(root.next));
	}

	void assign(size_type count, const_reference value)
	{
		PFL_ASSIGN(for (size_type i = 0; i < count; i++), value);
	}

	template<class InputIt>
	auto assign(InputIt first, InputIt last)
		-> std::enable_if_t<!std::is_integral_v<InputIt>>
	{
		PFL_ASSIGN(while (first != last), *first++);
	}

	auto front() noexcept -> reference
	{
		return root.next->ref;
	}
	auto front() const noexcept -> const_reference
	{
		return root.next->ref;
	}

	auto before_begin() noexcept -> iterator
	{
		return &root;
	}
	auto begin() noexcept -> iterator
	{
		return root.next;
	}
	auto end() noexcept -> iterator
	{
		return nullptr;
	}

	auto before_begin() const noexcept -> const_iterator
	{
		return const_cast<link *>(&root);
	}
	auto begin() const noexcept -> const_iterator
	{
		return root.next;
	}
	auto end() const noexcept -> const_iterator
	{
		return nullptr;
	}

	auto cbefore_begin() const noexcept -> const_iterator
	{
		return const_cast<link *>(&root);
	}
	auto cbegin() const noexcept -> const_iterator
	{
		return root.next;
	}
	auto cend() const noexcept -> const_iterator
	{
		return nullptr;
	}

	[[nodiscard]] auto empty() const noexcept -> bool
	{
		return !root.next;
	}

	auto max_size() const noexcept -> size_type
	{
		return std::numeric_limits<size_type>::max();
	}

	void clear() noexcept
	{
		PFL_FOR_ALL(root.next, PFL_ERASE(root.next));
	}

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

	template<class InputIt>
	auto insert_after(const_iterator pos, InputIt first, InputIt last)
		-> std::enable_if_t<!std::is_integral_v<InputIt>, iterator>
	{
		PFL_INSERT(while (first != last), *first++);
	}

	template<class Elem_Derived = Elem_Base, class ... Args>
	auto emplace_after(const_iterator pos, Args && ... args) -> iterator
	{
		return new node<Elem_Derived>(*pos.p, std::forward<Args>(args) ...);
	}

	auto erase_after(const_iterator pos) noexcept
	{
		PFL_ERASE(pos.p->next)
		return pos.p->next;
	}

	auto erase_after(const_iterator first, const_iterator last) noexcept
		-> iterator
	{
		PFL_FOR_ALL((first.p->next) != last.p, PFL_ERASE(first.p->next));
		return last.p;
	}

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
	auto emplace_front(Args && ... args) -> reference
	{
		basic_node * new_node =
			new node<Elem_Derived>(root, std::forward<Args>(args) ...);
		return new_node->ref;
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
		noexcept(noexcept(comp(other.root.next->ref, root.next->ref)))
	{
		PFL_MERGE(comp(other.root.next->ref, pivot->next->ref));
	}

	template<class Compare>
	void merge(polymorphic_forward_list && other, Compare comp)
		noexcept(noexcept(comp(other.root.next->ref, root.next->ref)))
	{
		PFL_MERGE(comp(other.root.next->ref, pivot->next->ref));
	}

	void splice_after(const_iterator pos, polymorphic_forward_list & other)
		noexcept
	{
		PFL_SPLICE_WHILE(pos.p, other.root.next, nullptr, pos.p->next);
	}

	void splice_after(const_iterator pos, polymorphic_forward_list && other)
		noexcept
	{
		PFL_SPLICE_WHILE(pos.p, other.root.next, nullptr, pos.p->next);
	}

	void splice_after(const_iterator pos, const_iterator it) noexcept
	{
		PFL_SPLICE(pos.p->next, it.p->next);
	}

	void splice_after(
		const_iterator pos,
		const_iterator first,
		const_iterator last) noexcept
	{
		PFL_SPLICE_WHILE(pos.p, first.p->next, last.p, pos.p->next != last.p);
	}

	void swap(polymorphic_forward_list & other) noexcept
	{
		PFL_SWAP(root.next, other.root.next);
	}

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
		PFL_FOR_ALL(root.next, PFL_SPLICE(reverse_root.next, root.next));
		root.next = reverse_root.next;
	}

private:
	link root;
};

template<class T>
auto operator==(
	polymorphic_forward_list<T> const & lhs,
	polymorphic_forward_list<T> const & rhs) noexcept -> bool
{
	auto left = lhs.begin();
	auto right = rhs.begin();
	auto end = lhs.end();
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
auto operator!=(
	polymorphic_forward_list<T> const & lhs,
	polymorphic_forward_list<T> const & rhs) noexcept -> bool
{
	auto left = lhs.begin();
	auto right = rhs.begin();
	auto end = lhs.end();
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
auto operator<(
	polymorphic_forward_list<T> const & lhs,
	polymorphic_forward_list<T> const & rhs) noexcept -> bool
{
	auto left = lhs.begin();
	auto right = rhs.begin();
	auto end = lhs.end();
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
auto operator>=(
	polymorphic_forward_list<T> const & lhs,
	polymorphic_forward_list<T> const & rhs) noexcept -> bool
{
	auto left = lhs.begin();
	auto right = rhs.begin();
	auto end = lhs.end();
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
auto operator>(
	polymorphic_forward_list<T> const & lhs,
	polymorphic_forward_list<T> const & rhs) noexcept -> bool
{
	auto left = lhs.begin();
	auto right = rhs.begin();
	auto end = lhs.end();
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
auto operator<=(
	polymorphic_forward_list<T> const & lhs,
	polymorphic_forward_list<T> const & rhs) noexcept -> bool
{
	auto left = lhs.begin();
	auto right = rhs.begin();
	auto end = lhs.end();
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

#undef PFL_ERASE
#undef PFL_SPLICE
#undef PFL_SPLICE_WHILE
#undef PFL_SWAP
#undef PFL_MERGE
#undef PFL_FOR_ALL
#undef PFL_ASSIGN
#undef PFL_INSERT
#undef PFL_REMOVE

#endif