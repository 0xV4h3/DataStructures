#pragma once
#include "SinglyLinkedList.hpp"

template<typename Value>
class Queue {
public:
	Queue();
	~Queue();

	void enqueue(Value& value);
	std::optional<std::reference_wrapper<Value>> dequeue();
	bool empty() const;
	size_t size() const;
	void clear();
private:
	SinglyLinkedList<Value> _list;
	mutable std::shared_mutex _queuemtx;
};

#include "Queue.cpp"