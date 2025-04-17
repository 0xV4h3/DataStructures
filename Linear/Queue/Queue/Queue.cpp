#include "Queue.hpp"

template<typename Value>
Queue<Value>::Queue() : _list() {}

template<typename Value>
Queue<Value>::~Queue() = default;

template<typename Value>
void Queue<Value>::enqueue(Value& value)
{
    std::unique_lock<std::shared_mutex> lock(_queuemtx);
    return _list.push_back(value);
}

template<typename Value>
std::optional<std::reference_wrapper<Value>> Queue<Value>::dequeue() {
    std::unique_lock<std::shared_mutex> lock(_queuemtx);
    auto result = _list.pop_front();
    if (result) {
        return std::ref(*result); 
    }
    return std::nullopt;
}

template<typename Value>
bool Queue<Value>::empty() const
{
    std::shared_lock<std::shared_mutex> lock(_queuemtx);
    return _list.empty();
}

template<typename Value>
size_t Queue<Value>::size() const
{
    std::shared_lock<std::shared_mutex> lock(_queuemtx);
    return _list.size();
}

template<typename Value>
void Queue<Value>::clear() {
    std::unique_lock<std::shared_mutex> lock(_queuemtx);
    _list.clear();
}
