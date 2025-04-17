#pragma once
#include "SinglyLinkedList.hpp"

template<typename Value>
class Stack {
public:
    Stack();
    ~Stack();
    void Push(const Value& value);
    std::optional<Value> Pop();
    std::optional<Value> Top() const;
    bool empty() const;
    size_t size() const;
    void clear();
private:
    SinglyLinkedList<Value> _list;
    mutable std::shared_mutex _stackmtx;
};

#include "Stack.cpp"