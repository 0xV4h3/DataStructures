#pragma once
#include "SinglyLinkedList.hpp"
#include <shared_mutex>
#include <optional>
#include <functional>

template<typename Value>
class Queue {
public:
    Queue() = default;
    ~Queue() = default;

    void enqueue(const Value& value) {
        std::unique_lock<std::shared_mutex> lock(_queuemtx);
        _list.push_back(value);
    }

    std::optional<Value> dequeue() {
        std::unique_lock<std::shared_mutex> lock(_queuemtx);
        return _list.pop_front();
    }

    bool empty() const {
        std::shared_lock lock(_queuemtx);
        return _list.empty();
    }

    size_t size() const {
        std::shared_lock lock(_queuemtx);
        return _list.size();
    }

    void clear() {
        std::unique_lock lock(_queuemtx);
        _list.clear();
    }

private:
    SinglyLinkedList<Value> _list;
    mutable std::shared_mutex _queuemtx;
};
