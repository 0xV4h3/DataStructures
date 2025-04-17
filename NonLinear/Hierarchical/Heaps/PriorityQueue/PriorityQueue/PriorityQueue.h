#pragma once
#include <memory>
#include <string>
#include "Heap.h"

template<typename T>
class PriorityQueue {
private:
    std::unique_ptr<Heap<T>> heap;

public:
    PriorityQueue(Heap<T>* h) : heap(h) {}

    void push(T value) {
        heap->push(value);
    }

    T pop() {
        return heap->pop();
    }

    T top() const {
        return heap->top();
    }

    bool empty() const {
        return heap->empty();
    }

    void visualize() const {
        heap->visualize();
    }

    std::string type() const {
        if (dynamic_cast<MinHeap<T>*>(heap.get())) {
            return "Ascending Queue";
        }
        else if (dynamic_cast<MaxHeap<T>*>(heap.get())) {
            return "Descending Queue";
        }
        else {
            return "Unknown Queue Type";
        }
    }
};

