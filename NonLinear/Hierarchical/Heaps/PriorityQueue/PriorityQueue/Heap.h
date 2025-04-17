#pragma once
#include <vector>
#include <stdexcept>
#include <algorithm>

template<typename T>
class Heap {
protected:
    std::vector<T> heap;

    size_t parent(size_t i) const { return (i == 0) ? 0 : (i - 1) / 2; }
    size_t left(size_t i) const { return 2 * i + 1; }
    size_t right(size_t i) const { return 2 * i + 2; }

    virtual bool compare(T a, T b) const = 0;

public:
    virtual ~Heap() = default;

    void push(T value);
    T pop();
    T top() const;
    bool empty() const;

    void visualize() const;
    void visualizeNode(size_t index, std::string prefix, bool isLeft) const;

protected:
    void heapifyDown(size_t i);
    void heapifyUp(size_t i);
};

template<typename T>
void Heap<T>::push(T value) {
    heap.push_back(value);
    heapifyUp(heap.size() - 1);
}

template<typename T>
T Heap<T>::pop() {
    if (heap.empty()) throw std::out_of_range("Heap is empty");
    T root = heap[0];
    heap[0] = heap.back();
    heap.pop_back();
    heapifyDown(0);
    return root;
}

template<typename T>
T Heap<T>::top() const {
    if (heap.empty()) throw std::out_of_range("Heap is empty");
    return heap[0];
}

template<typename T>
bool Heap<T>::empty() const {
    return heap.empty();
}

template<typename T>
void Heap<T>::heapifyDown(size_t i) {
    size_t l = left(i);
    size_t r = right(i);
    size_t extreme = i;

    if (l < heap.size() && compare(heap[l], heap[extreme]))
        extreme = l;
    if (r < heap.size() && compare(heap[r], heap[extreme]))
        extreme = r;

    if (extreme != i) {
        std::swap(heap[i], heap[extreme]);
        heapifyDown(extreme);
    }
}

template<typename T>
void Heap<T>::heapifyUp(size_t i) {
    while (i != 0 && compare(heap[i], heap[parent(i)])) {
        std::swap(heap[i], heap[parent(i)]);
        i = parent(i);
    }
}

template<typename T>
void Heap<T>::visualize() const {
    visualizeNode(0, "", false);
}

template<typename T>
void Heap<T>::visualizeNode(size_t index, std::string prefix, bool isLeft) const {
    if (index < heap.size()) {
        std::cout << prefix;
        std::cout << (isLeft ? "|-- " : "\\-- ");
        std::cout << heap[index] << std::endl;

        visualizeNode(left(index), prefix + (isLeft ? "|   " : "    "), true);
        visualizeNode(right(index), prefix + (isLeft ? "|   " : "    "), false);
    }
}

template<typename T>
class MinHeap : public Heap<T> {
protected:
    bool compare(T a, T b) const override {
        return a < b;
    }
};

template<typename T>
class MaxHeap : public Heap<T> {
protected:
    bool compare(T a, T b) const override {
        return a > b;
    }
};
