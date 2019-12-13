# `polymorphic_forward_list`
A node-based forward list with polymorphic node types

# Usage

`polymorphic_forward_list` is implemented in a single header. Copy the header file into your project's include directory,
or use whatever mechanism is provided by your build system. `#include "polymorphic_forward_list.hpp"` in any file where it is used.

# Interface

`polymorphic_forward_list` has an interface similar to that of `std::forward_list`.
Major exceptions are that it is neither allocator aware nor copyable.

# Use Cases

Use `polymorphic_forward_list<T>` instead of `std::forward_list<std::unique_ptr<T>>` when all owned objects are of type related to `T`.
Use it instead of `std::vector<std::unique_ptr<T>>` when a singly linked list is preferable to a vector.

For example, say we are implementing a gui framework where controls may have child controls whose dynamic types may not be
known until run time. Use `polymorphic_forward_list` to own and iterate over the child controls.
```cpp
class Control
{
  polymorphic_forward_list<Control> children;
...
```

# Design

`polymorphic_forward_list<T>` has nodes which own objects of various related types as subobjects. This is in contrast to a
`std::foward_list<std::unique_ptr<T>>` which owns objects through a pointer. This strategy eliminates an
indirection. As a side-effect, this strategy also allows owned objects to be destroyed directly. That is, an element is never
destroyed through a base pointer, and an object of type deriving from `T` need not have a virtual destructor in order to be an element.

An object of type `polymorphic_forward_list<T>` contains a single subobject of type `polymorphic_forward_list<T>::link`.
```cpp
template<class Base>
class polymorphic_forward_list
{
    link root;
...
```
An object of type `link` contains a single subobject of type `link *`, which points to the next link in the chain. A `link`
serves as a node without any reference to an object, so that the list can be implemented using the dummy node strategy.
```cpp
struct link
{
  link * next;
...
```
All node types inherit from `polymorphic_forward_list<T>::basic_node`, which is a `link` having a reference to the stored object
of type `reference to T`. A `basic_node` serves as the type through which nodes are accessed by iterators into the list.
```cpp
struct basic_node : link
{
  T & ref;
...
```
A node type is produced by instantiating a class template `polymorphic_forward_list<T>::node`.
An object of type `node<U>` is a `basic_node` containing a subobject of type `U` related to `T`. A `node` owns an element.
```cpp
template<class Derived>
struct node : basic_node
{
  Derived element;
...
``` 

# Todo

- Implement `polymorphic_forward_list::sort`
- Create tests
- Create documentation
