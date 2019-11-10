#ifndef POLYMORPHIC_FORWARD_LIST_HPP
#define POLYMORPHIC_FORWARD_LIST_HPP

#include <iterator>

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
			auto copy = *this;
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
			auto copy = *this;
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
		basic_node * temp = root.next;
		root.next = other.root.next;
		other.root.next = temp;
	}

	~polymorphic_forward_list() noexcept
	{
		while (root.next)
		{
			basic_node * trash = root.next;
			root.next = trash->next;
			delete trash;
		}
	}

	void assign(size_type count, const_reference value)
	{
		using node_elem_type =
			std::remove_const_t<std::remove_reference_t<const_reference>>;
		link assign_root = nullptr;
		link * assign_before_end = &assign_root;
		try
		{
			for (size_type i = 0; i < count; i++)
			{
				assign_before_end =
					new node<node_elem_type>(*assign_before_end, value);
			}
		}
		catch (...)
		{
			while (assign_root.next)
			{
				basic_node * trash = assign_root.next;
				assign_root.next = trash->next;
				delete trash;
			}
			throw;
		}
		while (root.next)
		{
			basic_node * trash = root.next;
			root.next = trash->next;
			delete trash;
		}
		root.next = assign_root.next;
	}

	template<class InputIt>
	auto assign(InputIt first, InputIt last)
		-> std::enable_if_t<!std::is_integral_v<InputIt>>
	{
		using node_elem_type =
			std::remove_const_t<std::remove_reference_t<decltype(*first)>>;
		link assign_root = nullptr;
		link * assign_before_end = &assign_root;
		try
		{
			while (first != last)
			{
				assign_before_end =
					new node<node_elem_type>(*assign_before_end, *first++);
			}
		}
		catch (...)
		{
			while (assign_root.next)
			{
				basic_node * trash = assign_root.next;
				assign_root.next = trash->next;
				delete trash;
			}
			throw;
		}
		while (root.next)
		{
			basic_node * trash = root.next;
			root.next = trash->next;
			delete trash;
		}
		root.next = assign_root.next;
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
		while (root.next)
		{
			basic_node * trash = root.next;
			root.next = trash->next;
			delete trash;
		}
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
		link splice_root;
		link * splice_before_end = &splice_root;
		try
		{
			for (size_type i = 0; i < count; i++)
			{
				splice_before_end
					= new node<Elem_Derived>(splice_before_end, value);
			}
		}
		catch (...)
		{
			while (splice_root.next)
			{
				basic_node * trash = splice_root.next;
				splice_root.next = trash->next;
				delete trash;
			}
			throw;
		}
		link * saved = pos.p->next;
		pos.p->next = splice_root.next;
		splice_before_end.p->next = saved;
		return splice_before_end;
	}

	template<class InputIt>
	auto insert_after(const_iterator pos, InputIt first, InputIt last)
		-> std::enable_if_t<!std::is_integral_v<InputIt>, iterator>
	{
		using node_elem_type =
			std::remove_const_t<std::remove_reference_t<decltype(*first)>>;
		link splice_root;
		link * splice_before_end = &splice_root;
		try
		{
			while (first != last)
			{
				splice_before_end =
					new node<node_elem_type>(splice_before_end, *first++);
			}
		}
		catch (...)
		{
			while (splice_root.next)
			{
				basic_node * trash = splice_root.next;
				splice_root.next = trash->next;
				delete trash;
			}
			throw;
		}
		link * saved = pos.p->next;
		pos.p->next = splice_root.next;
		splice_before_end.p->next = saved;
		return splice_before_end;
	}

	template<class Elem_Derived = Elem_Base, class ... Args>
	auto emplace_after(const_iterator pos, Args && ... args) -> iterator
	{
		return new node<Elem_Derived>(*pos.p, std::forward<Args>(args) ...);
	}

	auto erase_after(const_iterator pos) noexcept
	{
		basic_node * trash = pos.p->next;
		pos.p->next = trash->next;
		delete trash;
		return pos.p->next;
	}

	auto erase_after(const_iterator first, const_iterator last) noexcept
		-> iterator
	{
		while ((first.p->next) != last.p)
		{
			basic_node * trash = first.p->next;
			first.p->next = trash->next;
			delete trash;
		}
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
		if (this == &other) return;
		if (!other.root.next) return;
		link * pivot = &root;
		for (; pivot->next && other.root.next; pivot = pivot->next)
		{
			if (other.root.next->ref < pivot->next->ref)
			{
				basic_node * saved = pivot->next;
				pivot->next = other.root.next;
				other.root.next = pivot->next->next;
				pivot->next->next = saved; // that's a rotate
			}
		}
		if (other.root.next)
		{
			basic_node * saved = other.root.next;
			other.root.next = pivot->next;
			pivot->next = saved;
		}
	}

	void merge(polymorphic_forward_list && other)
		noexcept(noexcept(other.root.next->ref < root.next->ref))
	{
		if (this == &other) return;
		if (!other.root.next) return;
		link * pivot = &root;
		for (; pivot->next && other.root.next; pivot = pivot->next)
		{
			if (other.root.next->ref < pivot->next->ref)
			{
				basic_node * saved = pivot->next;
				pivot->next = other.root.next;
				other.root.next = pivot->next->next;
				pivot->next->next = saved;
			}
		}
		if (other.root.next)
		{
			basic_node * saved = other.root.next;
			other.root.next = pivot->next;
			pivot->next = saved;
		}
	}

	template<class Compare>
	void merge(polymorphic_forward_list & other, Compare comp)
		noexcept(noexcept(comp(other.root.next->ref, root.next->ref)))
	{
		if (this == &other) return;
		if (!other.root.next) return;
		link * pivot = &root;
		for (; pivot->next && other.root.next; pivot = pivot->next)
		{
			if (comp(other.root.next->ref, pivot->next->ref))
			{
				basic_node * saved = pivot->next;
				pivot->next = other.root.next;
				other.root.next = pivot->next->next;
				pivot->next->next = saved;
			}
		}
		if (other.root.next)
		{
			basic_node * saved = other.root.next;
			other.root.next = pivot->next;
			pivot->next = saved;
		}
	}

	template<class Compare>
	void merge(polymorphic_forward_list && other, Compare comp)
		noexcept(noexcept(comp(other.root.next->ref, root.next->ref)))
	{
		if (this == &other) return;
		if (!other.root.next) return;
		link * pivot = &root;
		for (; pivot->next && other.root.next; pivot = pivot->next)
		{
			if (comp(other.root.next->ref, pivot->next->ref))
			{
				basic_node * temp = pivot->next;
				pivot->next = other.root.next;
				other.root.next = pivot->next->next;
				pivot->next->next = temp;
			}
		}
		if (other.root.next)
		{
			basic_node * saved = other.root.next;
			other.root.next = pivot->next;
			pivot->next = saved;
		}
	}

	void splice_after(const_iterator pos, polymorphic_forward_list & other)
		noexcept
	{
		basic_node * saved = pos.p->next;
		pos.p->next = other.root.next;
		other.root.next = nullptr;
		while (pos.p->next) pos.p = pos.p->next;
		pos.p->next = saved;
	}

	void splice_after(const_iterator pos, polymorphic_forward_list && other)
		noexcept
	{
		basic_node * saved = pos.p->next;
		pos.p->next = other.root.next;
		other.root.next = nullptr;
		while (pos.p->next) pos.p = pos.p->next;
		pos.p->next = saved;
	}

	void splice_after(const_iterator pos, const_iterator it) noexcept
	{
		basic_node * saved = pos.p->next;
		pos.p->next = it.p->next;
		it.p->next = pos.p->next->next;
		pos.p->next->next = saved;
	}

	void splice_after(
		const_iterator pos,
		const_iterator first,
		const_iterator last) noexcept
	{
		basic_node * saved = pos.p->next;
		pos.p->next = first.p->next;
		first.p->next = last.p;
		while (pos.p->next != last.p) pos.p = pos.p->next;
		pos.p->next = saved;
	}

	void swap(polymorphic_forward_list & other) noexcept
	{
		basic_node * saved = root.next;
		root.next = other.root.next;
		other.root.next = saved;
	}

	auto remove(const_reference value) -> size_type
	{
		size_type removed_count = 0;
		for (link * pivot = &root; pivot->next; pivot = pivot->next)
		{
			if (pivot->next->ref == value)
			{
				basic_node * trash = pivot->next;
				pivot->next = trash->next;
				delete trash;
				removed_count++;
			}
		}
		return removed_count;
	}

	template<class UnaryPredicate>
	auto remove_if(UnaryPredicate p) -> size_type
	{
		size_type removed_count = 0;
		for (link * pivot = &root; pivot->next; pivot = pivot->next)
		{
			if (p(pivot->next->ref))
			{
				basic_node * trash = pivot->next;
				pivot->next = trash->next;
				delete trash;
				removed_count++;
			}
		}
		return removed_count;
	}

	void reverse() noexcept
	{
		link reverse_root = nullptr;
		while (root.next)
		{
			basic_node * saved = reverse_root.next;
			reverse_root.next = root.next;
			root.next = reverse_root.next->next;
			reverse_root.next->next = saved;
		}
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

#endif