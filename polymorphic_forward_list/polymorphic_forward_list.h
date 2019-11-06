#pragma once

#include <memory>

template<class Elem_Base>
class polymorphic_forward_list
{
	template<bool is_const>
	class iterator_t;

public:	
	using value_type = Elem_Base;
	using size_type = size_t;
	using difference_type = ptrdiff_t;
	using reference = value_type &;
	using const_reference = value_type const &;
	using pointer = value_type *;
	using const_pointer = value_type const *;
	using iterator = iterator_t<false>;
	using const_iterator = iterator_t<true>;

private:

	struct basic_node;

	struct link
	{
		basic_node * next = nullptr;

		link(basic_node * next) :
			next{ next }
		{ }
	};

	struct basic_node : link
	{
		basic_node(basic_node * next, reference ref) :
			link{ next },
			ref{ ref }
		{ }

		reference ref;
		virtual ~basic_node() noexcept = default;
	};

	template<class Elem_Derived>
	struct node : basic_node
	{
		template<class ... Args>
		node(basic_node * next, Args && ... args) noexcept(noexcept(Elem_Derived{ std::forward<Args>(args) ... })) :
			basic_node{ next, elem },
			elem{ std::forward<Args>(args) ... }
		{ }

		Elem_Derived elem;
	};
	
	template<bool is_const>
	class iterator_t
	{
		friend class polymorphic_forward_list;

	public:
		using difference_type = void;
		using value_type = value_type;
		using pointer = std::conditional_t<is_const, const_pointer, pointer>;
		using reference = std::conditional_t<is_const, const_reference, reference>;
		using iterator_category = std::forward_iterator_tag;

		iterator_t() noexcept = default;
		iterator_t(iterator_t const &) noexcept = default;
		auto operator=(iterator_t const &) noexcept -> iterator_t & = default;

		reference operator*() const noexcept
		{
			return static_cast<basic_node *>(p)->ref;
		}
		pointer operator->() const noexcept
		{
			return &static_cast<basic_node *>(p)->ref;
		}

		iterator_t & operator++() noexcept
		{
			p = p->next;
			return *this;
		}
		iterator_t operator++(int) noexcept
		{
			auto copy = *this;
			p = p->next;
			return copy;
		}

		bool operator!=(iterator_t const & other) noexcept
		{
			return p != other.p;
		}
		bool operator==(iterator_t const & other) noexcept
		{
			return p == other.p;
		}

		operator const_iterator() const
		{
			return p;
		}

	private:
		link * p = nullptr;

		iterator_t(link * p) noexcept :
			p{ p }
		{ }

		operator link * () noexcept
		{
			return p;
		}
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
		std::swap(root.next, other.root.next);
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
		using other_type = std::remove_const_t<std::remove_reference_t<decltype(*first)>>;
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
		return nullptr;
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
		return nullptr;
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
		return nullptr;
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
		erase_after(&root, nullptr);
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
		delete trash;
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

	template<class Compare>
	void merge(polymorphic_forward_list & other, Compare comp) noexcept(noexcept(comp(root.next->ref, root.next->ref)))
	{
		if (this == &other) return;
		if (!other.root.next) return;
		link * left = &root;
        for (; other.root.next && left->next; left = left->next)
		{
			if (comp(other.root.next->ref, left->next->ref))
			{
				std::swap(other.root.next, left->next);
				std::swap(other.root.next, left->next->next);
			}
        }
		std::swap(left->next, other.root.next);
	}
		
	void merge(polymorphic_forward_list & other) noexcept(noexcept(root.next->ref < root.next->ref))
	{
		return merge(other, std::less{});
	}

	void merge(polymorphic_forward_list && other) noexcept(noexcept(root.next->ref < root.next->ref))
	{
		return merge(other, std::less{});
	}

	template<class Compare>
	void merge(polymorphic_forward_list && other, Compare comp) noexcept(noexcept(merge(other, std::move(comp))))
	{
		return merge(other, std::move(comp));
	}

	void splice_after(const_iterator pos, polymorphic_forward_list & other) noexcept
	{
		std::swap(pos.p->next, other.root.next);
		while (pos.p->next) pos++;
		std::swap(pos.p->next, other.root.next);
	}

	void splice_after(const_iterator pos, polymorphic_forward_list && other) noexcept
	{
		splice_after(pos, other);
	}

	void swap(polymorphic_forward_list & other) noexcept
	{
		std::swap(root.next, other.root.next);
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
