#include "Stack.hpp"

template<typename Value>
Stack<Value>::Stack() : _list() {}

template<typename Value>
Stack<Value>::~Stack() = default;

template<typename Value>
void Stack<Value>::Push(const Value& value)
{
    std::unique_lock<std::shared_mutex> lock(_stackmtx);
    _list.push_front(value);
}

template<typename Value>
std::optional<Value> Stack<Value>::Pop()
{
    std::unique_lock<std::shared_mutex> lock(_stackmtx);
    if (_list.empty())
        return std::nullopt;
    return _list.pop_front();
}

template<typename Value>
std::optional<Value> Stack<Value>::Top() const
{
    std::shared_lock<std::shared_mutex> lock(_stackmtx);
    if (_list.empty())
        return std::nullopt;
    return _list.front();
}

template<typename Value>
bool Stack<Value>::empty() const
{
    std::shared_lock<std::shared_mutex> lock(_stackmtx);
    return _list.empty();
}

template<typename Value>
size_t Stack<Value>::size() const
{
    std::shared_lock<std::shared_mutex> lock(_stackmtx);
    return _list.size();
}