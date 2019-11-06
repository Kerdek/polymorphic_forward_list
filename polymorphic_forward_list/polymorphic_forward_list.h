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
	};

	struct basic_node : link
	{
		basic_node(reference ref) :
			ref{ ref }
		{ }

		reference ref;
		virtual ~basic_node() noexcept = default;
	};

	template<class Elem_Derived>
	struct node : basic_node
	{
		template<class ... Args>
		node(Args && ... args) noexcept(noexcept(Elem_Derived{ std::forward<Args>(args) ... })) :
			basic_node{ elem },
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
		iterator_t(link * p) noexcept :
			p{ p }
		{ }

		link * p = nullptr;
	};

public:

	polymorphic_forward_list() noexcept
	{}

	polymorphic_forward_list(polymorphic_forward_list &&) noexcept
	{
		root.next = other.root.next;
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

	void assign(size_type count, const_reference value) // needs to be optimized
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
		link * last_inserted = &root;
		while (first != last)
		{
			last_inserted->next = std::make_unique<node<other_type>>(*first);
			last_inserted = last_inserted->next.get();
			++first;
		}
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
		return root.next.get();
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
		return root.next.get();
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
		return root.next.get();
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
		while (root.next)
		{
			std::unique_ptr<basic_node> trash = std::move(root.next);
			std::swap(trash->next, root.next);
		}
	}

	template<class Elem_Derived>
	iterator insert_after(const_iterator pos, Elem_Derived const & value)
	{
		std::unique_ptr<basic_node> new_node = std::make_unique<node<Elem_Derived>>(value);
		std::swap(pos.p->next, new_node->next);
		std::swap(pos.p->next, new_node);
		return pos.p->next.get();
	}

	template<class Elem_Derived>
	iterator insert_after(const_iterator pos, Elem_Derived && value)
	{
		std::unique_ptr<basic_node> new_node = std::make_unique<node<Elem_Derived>>(std::move(value));
		std::swap(pos.p->next, new_node->next);
		std::swap(pos.p->next, new_node);
		return pos.p->next.get();
	}

	template<class Elem_Derived>
	iterator insert_after(const_iterator pos, size_type count, Elem_Derived const & value) // needs to be optimized
	{
		for (size_type i = 0; i < count; i++)
		{
			insert_after<value_type>(pos, value);
		}
	}

	template<class InputIt>
	iterator insert_after(const_iterator pos, InputIt first, InputIt last)
	{
		using other_type = std::remove_const_t<std::remove_reference_t<decltype(*first)>>;
		link * last_inserted = pos.p;
		while (first != last)
		{
			last_inserted->next = std::make_unique<node<other_type>>(*first);
			last_inserted = last_inserted->next.get();
			++first;
		}
		return last_inserted;
	}

	template<class Elem_Derived = Elem_Base, class ... Args>
	iterator emplace_after(const_iterator pos, Args && ... args)
	{
		std::unique_ptr<basic_node> new_node = std::make_unique<node<Elem_Derived>>(std::forward<Args>(args) ...);
		std::swap(pos.p->next, new_node->next);
		std::swap(pos.p->next, new_node);
		return pos.p->next.get();
	}

	iterator erase_after(const_iterator pos)
	{
		std::unique_ptr<basic_node> trash = std::move(pos.p->next);
		std::swap(pos.p->next, trash->next);
		return pos.p->next.get();
	}
	
	iterator erase_after(const_iterator first, const_iterator last)
	{
		while (first != last)
		{
			std::unique_ptr<basic_node> trash = std::move(first.p->next);
			std::swap(first.p->next, trash->next);
		}
		return last.p;
	}

	template<class Elem_Derived>
	void push_front(Elem_Derived const & value)
	{
		std::unique_ptr<basic_node> new_head = std::make_unique<node<Elem_Derived>>(value);
		std::swap(root.next, new_head->next);
		std::swap(root.next, new_head);
	}

	template<class Elem_Derived>
	void push_front(Elem_Derived && value)
	{
		std::unique_ptr<basic_node> new_head = std::make_unique<node<Elem_Derived>>(std::move(value));
		std::swap(root.next, new_head->next);
		std::swap(root.next, new_head);
	}

	template<class Elem_Derived = Elem_Base, class ... Args>
	reference emplace_front(Args && ... args)
	{
		std::unique_ptr<basic_node> new_head = std::make_unique<node<Elem_Derived>>(std::forward<Args>(args) ...);
		std::swap(root.next, new_head->next);
		std::swap(root.next, new_head);
		return root.next->ref;
	}

	template<class Compare>
	void merge(polymorphic_forward_list & other, Compare comp)
	{
		if (this == &other) return;
		if (!other.root.next) return;
		link * left = &root;
        for (; left->next && other.root.next; left = left->next.get())
		{
			if (comp(other.root.next->ref, left->next->ref))
			{
				std::swap(left->next, other.root.next);
				std::swap(left->next->next, other.root.next);
			}
        }
		std::swap(left->next, other.root.next);
	}
		
	void merge(polymorphic_forward_list & other)
	{
		return merge(other, std::less{});
	}

	void merge(polymorphic_forward_list && other)
	{
		return merge(other, std::less{});
	}

	template<class Compare>
	void merge(polymorphic_forward_list && other, Compare comp)
	{
		return merge(other, std::move(comp));
	}

	void splice_after(const_iterator pos, polymorphic_forward_list & other)
	{
		link * it = pos.p;
		std::swap(it->next, other.root.next); // swap our tail with the other list
		while (it->next) it = it->next.get(); // fast forward to the end of the list
		std::swap(it->next, other.root.next); // swap the other list onto our tail
	}

	void splice_after(const_iterator pos, polymorphic_forward_list && other)
	{
		splice_after(pos, other);
	}

	void swap(polymorphic_forward_list & other)
	{
		std::swap(root.next, other.root.next);
	}

private:
	link root;
};

template<class T>
bool operator==(polymorphic_forward_list<T> const & lhs, polymorphic_forward_list<T> const & rhs)
{
	return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}
template<class T>
bool operator!=(polymorphic_forward_list<T> const & lhs, polymorphic_forward_list<T> const & rhs)
{
	return !operator==(lhs, rhs);
}
template<class T>
bool operator<(polymorphic_forward_list<T> const & lhs, polymorphic_forward_list<T> const & rhs)
{
	return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), std::less{});
}
template<class T>
bool operator>=(polymorphic_forward_list<T> const & lhs, polymorphic_forward_list<T> const & rhs)
{
	return !operator<(lhs, rhs);
}
template<class T>
bool operator>(polymorphic_forward_list<T> const & lhs, polymorphic_forward_list<T> const & rhs)
{
	return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), std::greater{});
}
template<class T>
bool operator<=(polymorphic_forward_list<T> const & lhs, polymorphic_forward_list<T> const & rhs)
{
	return !operator>(lhs, rhs);
}
