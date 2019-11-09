#pragma once

#include <iterator>

template<class Elem_Base>
class polymorphic_forward_list
{
	class link;
	
public:
	using value_type = Elem_Base;
	using size_type = size_t;
	using reference = value_type &;
	using const_reference = value_type const &;
	using pointer = value_type *;
	using difference_type = void;
	using const_pointer = value_type const *;

	class iterator;
	class const_iterator;

	class iterator
	{
		friend class polymorphic_forward_list;

	public:
		using difference_type = void;
		using value_type = value_type;
		using pointer = pointer;
		using reference = reference;
		using iterator_category = std::forward_iterator_tag;

		iterator() noexcept = default;
		iterator(iterator const &) noexcept = default;
		auto operator=(iterator const &) noexcept->iterator & = default;

		reference operator*() const noexcept
		{
			return static_cast<basic_node &>(*p).ref;
		}
		pointer operator->() const noexcept
		{
			return &static_cast<basic_node &>(*p).ref;
		}

		iterator & operator++() noexcept
		{
			p = p->next;
			return *this;
		}
		iterator operator++(int) noexcept
		{
			auto copy = *this;
			p = p->next;
			return copy;
		}

		bool operator!=(iterator const & other) noexcept
		{
			return p != other.p;
		}
		bool operator==(iterator const & other) noexcept
		{
			return p == other.p;
		}

		operator const_iterator()
		{
			return p;
		}

	private:
		link * p = nullptr;

		iterator(const_iterator const & other) :
			p{ other.p }
		{ }
		iterator(link * l) noexcept :
			p{ l }
		{ }

		operator link * () noexcept
		{
			return p;
		}
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

		const_iterator() noexcept = default;
		const_iterator(const_iterator const &) noexcept = default;
		auto operator=(const_iterator const &) noexcept->const_iterator & = default;

		reference operator*() const noexcept
		{
			return static_cast<basic_node &>(*p).ref;
		}
		pointer operator->() const noexcept
		{
			return &static_cast<basic_node &>(*p).ref;
		}

		const_iterator & operator++() noexcept
		{
			p = p->next;
			return *this;
		}
		const_iterator operator++(int) noexcept
		{
			auto copy = *this;
			p = p->next;
			return copy;
		}

		bool operator!=(const_iterator const & other) noexcept
		{
			return p != other.p;
		}
		bool operator==(const_iterator const & other) noexcept
		{
			return p == other.p;
		}

	private:
		link * p = nullptr;

		const_iterator(link * l) noexcept :
			p{ l }
		{ }

		operator link * () noexcept
		{
			return p;
		}
	};

private:

	class basic_node;

	struct link
	{
		link() = delete;
		link(link const &) = delete;
		link(link &&) = delete;
		link & operator=(link const &) = delete;
		link & operator=(link &&) = delete;
		~link() = default;

		link(basic_node * next) :
			next{ next }
		{ }

		basic_node * next;
	};

	struct basic_node : link
	{
		basic_node(basic_node * next, reference ref) :
			link{ next },
			ref{ ref }
		{ }

		virtual ~basic_node() noexcept = default;

		reference ref;
	};

	template<class Elem_Derived>
	struct basic_owner
	{
		template<class ... Args>
		basic_owner(Args && ... args) noexcept(noexcept(Elem_Derived{ std::forward<Args>(args) ... })) :
			elem{ std::forward<Args>(args) ... }
		{ }

		Elem_Derived elem;
	};

	template<class Elem_Derived>
	struct node : basic_owner<Elem_Derived>, basic_node
	{
		template<class ... Args>
		node(basic_node * next, Args && ... args) noexcept(noexcept(Elem_Derived{ std::forward<Args>(args) ... })) :
			basic_owner<Elem_Derived>{ std::forward<Args>(args) ... },
			basic_node{ next, basic_owner<Elem_Derived>::elem }
		{ }
	};

public:

	polymorphic_forward_list() noexcept :
		root{ nullptr }
	{}

	polymorphic_forward_list(polymorphic_forward_list && other) noexcept :
		root{ other.root.next }
	{
		other.root.next = nullptr;
	}
	auto operator=(polymorphic_forward_list && other) noexcept -> polymorphic_forward_list &
	{
		basic_node * temp = root.next;
		root.next = other.root.next;
		other.root.next = temp;
	}

	~polymorphic_forward_list() noexcept
	{
		clear();
	}

	void assign(size_type count, const_reference value)
	{
		clear();
		for (size_type i = 0; i < count; i++)
		{
			emplace_front<value_type>(value);
		}
	}

	template<class InputIt>
	void assign(InputIt first, InputIt last)
	{
		clear();
		insert_after(&root, first, last);
	}

	reference front() noexcept
	{
		return root.next->ref;
	}
	const_reference front() const noexcept
	{
		return root.next->ref;
	}

	iterator before_begin() noexcept
	{
		return &root;
	}
	iterator begin() noexcept
	{
		return root.next;
	}
	iterator end() noexcept
	{
		return {};
	}

	const_iterator before_begin() const noexcept
	{
		return const_cast<link *>(&root); // This is okay because functions which modify the container will never be called through a const reference
	}
	const_iterator begin() const noexcept
	{
		return root.next;
	}
	const_iterator end() const noexcept
	{
		return {};
	}

	const_iterator cbefore_begin() const noexcept
	{
		return const_cast<link *>(&root); // This is okay because functions which modify the container will never be called through a const reference
	}
	const_iterator cbegin() const noexcept
	{
		return root.next;
	}
	const_iterator cend() const noexcept
	{
		return {};
	}

	[[nodiscard]] bool empty() const noexcept
	{
		return !root.next;
	}

	size_type max_size() const noexcept
	{
		return std::numeric_limits<size_type>::max();
	}

	void clear() noexcept
	{
		erase_after(&root, end());
	}

	template<class Elem_Derived>
	iterator insert_after(const_iterator pos, Elem_Derived const & value)
	{
		return pos.p->next = new node<Elem_Derived>(pos.p->next, value);
	}

	template<class Elem_Derived>
	iterator insert_after(const_iterator pos, Elem_Derived && value)
	{
		return pos.p->next = new node<Elem_Derived>(pos.p->next, std::move(value));
	}

	template<class Elem_Derived>
	iterator insert_after(const_iterator pos, size_type count, Elem_Derived const & value)
	{
		polymorphic_forward_list splice;
		auto it = splice.before_begin();
		for (size_type i = 0; i < count; i++)
		{
			it = splice.insert_after<Elem_Derived>(it, value);
		}
		splice_after(pos, splice);
		return it;
	}

	template<class InputIt>
	iterator insert_after(const_iterator pos, InputIt first, InputIt last)
	{
		polymorphic_forward_list splice;
		auto it = splice.before_begin();
		while (first != last)
		{
			it = splice.insert_after<std::remove_const_t<std::remove_reference_t<decltype(*first)>>>(it, *first++);
		}
		splice_after(pos, splice);
		return it;
	}

	template<class Elem_Derived = Elem_Base, class ... Args>
	iterator emplace_after(const_iterator pos, Args && ... args)
	{
		return pos.p->next = new node<Elem_Derived>(pos.p->next, std::forward<Args>(args) ...);
	}

	iterator erase_after(const_iterator pos) noexcept
	{
		basic_node * trash = pos.p->next;
		pos.p->next = trash->next;
		return pos.p->next;
	}

	iterator erase_after(const_iterator first, const_iterator last) noexcept
	{
		while ((first.p->next) != last)
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
		root.next = new node<Elem_Derived>(root.next, value);
	}

	template<class Elem_Derived>
	void push_front(Elem_Derived && value)
	{
		root.next = new node<Elem_Derived>(root.next, std::move(value));
	}

	template<class Elem_Derived = Elem_Base, class ... Args>
	reference emplace_front(Args && ... args)
	{
		root.next = new node<Elem_Derived>(root.next, std::forward<Args>(args) ...);
		return root.next->ref;
	}

	void merge(polymorphic_forward_list & other) noexcept(noexcept(root.next->ref < root.next->ref))
	{
		merge(other, std::less{});
	}

	void merge(polymorphic_forward_list && other) noexcept(noexcept(root.next->ref < root.next->ref))
	{
		merge(other, std::less{});
	}

	template<class Compare>
	void merge(polymorphic_forward_list & other, Compare comp) noexcept(noexcept(comp(root.next->ref, root.next->ref)))
	{
		if (this == &other) return;
		if (!other.root.next) return;
		link * pivot = &root;
		for (; pivot->next && other.root.next; pivot = pivot->next)
		{
			if (comp(other.root.next->ref, pivot->next->ref))
			{
				splice_after(pivot, &other.root);
			}
		}
		if (other.root.next)
		{
			basic_node * temp = other.root.next;
			other.root.next = pivot->next;
			pivot->next = temp;
		}
	}

	template<class Compare>
	void merge(polymorphic_forward_list && other, Compare comp) noexcept(noexcept(merge(other, std::move(comp))))
	{
		merge(other, std::move(comp));
	}

	void splice_after(const_iterator pos, polymorphic_forward_list & other) noexcept
	{
		splice_after(pos, other.before_begin(), other.end());
	}

	void splice_after(const_iterator pos, polymorphic_forward_list && other) noexcept
	{
		splice_after(pos, other);
	}

	void splice_after(const_iterator pos, const_iterator it) noexcept
	{
		basic_node * temp = pos.p->next;
		pos.p->next = it.p->next;
		it.p->next = it.p->next->next;
		pos.p->next->next = temp;
	}

	void splice_after(const_iterator pos, const_iterator first, const_iterator last) noexcept
	{
		basic_node * temp = pos.p->next;
		pos.p->next = first.p->next;
		while (pos.p->next != last) ++pos;
		first.p->next = pos.p->next;
		pos.p->next = temp;
	}

	void swap(polymorphic_forward_list & other) noexcept
	{
		basic_node * temp = root.next;
		root.next = other.root.next;
		other.root.next = temp;
	}

private:
	link root;
};

template<class T>
bool operator==(polymorphic_forward_list<T> const & lhs, polymorphic_forward_list<T> const & rhs) noexcept
{
	return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}
template<class T>
bool operator!=(polymorphic_forward_list<T> const & lhs, polymorphic_forward_list<T> const & rhs) noexcept
{
	return !operator==(lhs, rhs);
}
template<class T>
bool operator<(polymorphic_forward_list<T> const & lhs, polymorphic_forward_list<T> const & rhs) noexcept
{
	return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), std::less{});
}
template<class T>
bool operator>=(polymorphic_forward_list<T> const & lhs, polymorphic_forward_list<T> const & rhs) noexcept
{
	return !operator<(lhs, rhs);
}
template<class T>
bool operator>(polymorphic_forward_list<T> const & lhs, polymorphic_forward_list<T> const & rhs) noexcept
{
	return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), std::greater{});
}
template<class T>
bool operator<=(polymorphic_forward_list<T> const & lhs, polymorphic_forward_list<T> const & rhs) noexcept
{
	return !operator>(lhs, rhs);
}
