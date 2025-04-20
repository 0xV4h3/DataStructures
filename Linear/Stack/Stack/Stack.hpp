#pragma once
#include "SinglyLinkedList.hpp"

template<typename Value>
class Stack {
public:
    Stack() : _list() {}

    ~Stack() = default;

    void Push(const Value& value) {
        std::unique_lock<std::shared_mutex> lock(_stackmtx);
        _list.push_front(value);
    }

    std::optional<Value> Pop() {
        std::unique_lock<std::shared_mutex> lock(_stackmtx);
        if (_list.empty())
            return std::nullopt;
        return _list.pop_front();
    }

    std::optional<Value> Top() const {
        std::shared_lock<std::shared_mutex> lock(_stackmtx);
        if (_list.empty())
            return std::nullopt;
        return _list.front();
    } 

    bool empty() const {
        std::shared_lock<std::shared_mutex> lock(_stackmtx);
        return _list.empty();
    }

    size_t size() const {
        std::shared_lock<std::shared_mutex> lock(_stackmtx);
        return _list.size();
    }

    void clear() {
        std::unique_lock<std::shared_mutex> lock(_stackmtx);
        _list.clear();
    }

private:
    SinglyLinkedList<Value> _list;
    mutable std::shared_mutex _stackmtx;
};