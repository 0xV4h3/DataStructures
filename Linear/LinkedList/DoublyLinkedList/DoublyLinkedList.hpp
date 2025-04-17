#pragma once
#include <iostream>
#include <optional>
#include <memory>
#include <shared_mutex>
#include <mutex>
#include <utility>
#include <stdexcept>
#include <functional>

template<typename Key, typename Value>
class DoublyLinkedList {
private:
    struct Node {
        std::pair<Key, Value> data;
        std::shared_ptr<Node> next;
        std::weak_ptr<Node> prev;
        Node(const Key& key, const Value& value) : data(key, value), next(nullptr), prev() {}
    };

    std::shared_ptr<Node> head;
    std::shared_ptr<Node> tail;
    mutable std::shared_mutex mtx;
    size_t count = 0;

    // Helpers (non thread-safe)

    void clear_internal() {
        head.reset();
        tail.reset();
        count = 0;
    }

    void push_front_internal(const Key& key, const Value& value) {
        auto new_node = std::make_shared<Node>(key, value);
        new_node->next = head;
        if (head) {
            head->prev = new_node;
        }
        head = new_node;
        if (!tail) {
            tail = new_node;
        }
        ++count;
    }

    void push_back_internal(const Key& key, const Value& value) {
        auto new_node = std::make_shared<Node>(key, value);
        new_node->prev = tail;
        if (!tail) {
            head = tail = new_node;
        }
        else {
            tail->next = new_node;
            tail = new_node;
        }
        ++count;
    }

    std::optional<std::pair<Key, Value>> pop_front_internal() {
        if (!head)
            return std::nullopt;
        auto result = head->data;
        head = head->next;
        if (head) {
            head->prev.reset();
        }
        else {
            tail.reset();
        }
        --count;
        return result;
    }

    std::optional<std::pair<Key, Value>> pop_back_internal() {
        if (!tail)
            return std::nullopt;
        auto result = tail->data;
        auto prev = tail->prev.lock();
        if (prev) {
            prev->next.reset();
            tail = prev;
        }
        else {
            head.reset();
            tail.reset();
        }
        --count;
        return result;
    }

public:
    // Iterator for non-const access
    class iterator {
        friend class DoublyLinkedList;
    public:
        using value_type = std::pair<Key, Value>;
        using pointer = value_type*;
        using reference = value_type&;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::bidirectional_iterator_tag;

        iterator(Node* ptr, Node* tailPtr) : current(ptr), tail(tailPtr) {}

        reference operator*() const { return current->data; }
        pointer operator->() const { return &(current->data); }

        // Prefix increment
        iterator& operator++() {
            if (current)
                current = current->next.get();
            return *this;
        }
        // Postfix increment
        iterator operator++(int) {
            iterator temp = *this;
            ++(*this);
            return temp;
        }
        // Prefix decrement
        iterator& operator--() {
            if (!current) {
                current = tail;
            }
            else {
                current = current->prev.lock().get();
            }
            return *this;
        }
        // Postfix decrement
        iterator operator--(int) {
            iterator temp = *this;
            --(*this);
            return temp;
        }
        bool operator==(const iterator& other) const { return current == other.current; }
        bool operator!=(const iterator& other) const { return current != other.current; }

    private:
        Node* current;
        Node* tail;
    };

    // Iterator for const access
    class const_iterator {
        friend class DoublyLinkedList;
    public:
        using value_type = const std::pair<Key, Value>;
        using pointer = const value_type*;
        using reference = const value_type&;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::bidirectional_iterator_tag;

        const_iterator(const Node* ptr, const Node* tailPtr) : current(ptr), tail(tailPtr) {}

        reference operator*() const { return current->data; }
        pointer operator->() const { return &(current->data); }

        const_iterator& operator++() {
            if (current)
                current = current->next.get();
            return *this;
        }
        const_iterator operator++(int) {
            const_iterator temp = *this;
            ++(*this);
            return temp;
        }
        const_iterator& operator--() {
            if (!current)
                current = tail;
            else
                current = current->prev.lock().get();
            return *this;
        }
        const_iterator operator--(int) {
            const_iterator temp = *this;
            --(*this);
            return temp;
        }
        bool operator==(const const_iterator& other) const { return current == other.current; }
        bool operator!=(const const_iterator& other) const { return current != other.current; }

    private:
        const Node* current;
        const Node* tail;
    };

    // begin/end functions for iterator support
    iterator begin() {
        return iterator(head.get(), tail.get());
    }
    iterator end() {
        return iterator(nullptr, tail.get());
    }
    const_iterator begin() const {
        return const_iterator(head.get(), tail.get());
    }
    const_iterator end() const {
        return const_iterator(nullptr, tail.get());
    }
    const_iterator cbegin() const {
        return const_iterator(head.get(), tail.get());
    }
    const_iterator cend() const {
        return const_iterator(nullptr, tail.get());
    }

    // Default constructor
    DoublyLinkedList() : head(nullptr), tail(nullptr), count(0) {}

    // Copy constructor (deep copy)
    DoublyLinkedList(const DoublyLinkedList& other) {
        std::shared_lock lock(other.mtx);
        clear_internal();
        auto current = other.head;
        while (current) {
            push_back(current->data.first, current->data.second);
            current = current->next;
        }
    }

    // Move constructor
    DoublyLinkedList(DoublyLinkedList&& other) noexcept {
        std::unique_lock lock(other.mtx);
        head = std::move(other.head);
        tail = std::move(other.tail);
        count = other.count;
        other.count = 0;
    }

    // Copy assignment operator 
    DoublyLinkedList& operator=(const DoublyLinkedList& other) {
        if (this != &other) {
            std::unique_lock lock1(mtx, std::defer_lock);
            std::shared_lock lock2(other.mtx, std::defer_lock);
            std::lock(lock1, lock2);
            clear_internal();
            if (other.head) {
                head = std::make_shared<Node>(other.head->data.first, other.head->data.second);
                tail = head;
                count = 1;
                auto current_other = other.head->next;
                while (current_other) {
                    auto new_node = std::make_shared<Node>(current_other->data.first, current_other->data.second);
                    new_node->prev = tail;
                    tail->next = new_node;
                    tail = new_node;
                    ++count;
                    current_other = current_other->next;
                }
            }
        }
        return *this;
    }

    // Move assignment operator
    DoublyLinkedList& operator=(DoublyLinkedList&& other) noexcept {
        if (this != &other) {
            std::unique_lock lock1(mtx, std::defer_lock);
            std::unique_lock lock2(other.mtx, std::defer_lock);
            std::lock(lock1, lock2);
            head = std::move(other.head);
            tail = std::move(other.tail);
            count = other.count;
            other.count = 0;
        }
        return *this;
    }

    // Destructor
    ~DoublyLinkedList() {
        std::unique_lock lock(mtx);
        clear_internal();
    }

    // Modifiers

    void push_front(const Key& key, const Value& value) {
        std::unique_lock lock(mtx);
        push_front_internal(key, value);
    }

    void push_back(const Key& key, const Value& value) {
        std::unique_lock lock(mtx);
        push_back_internal(key, value);
    }

    std::optional<std::pair<Key, Value>> pop_front() {
        std::unique_lock lock(mtx);
        return pop_front_internal();
    }

    std::optional<std::pair<Key, Value>> pop_back() {
        std::unique_lock lock(mtx);
        return pop_back_internal();
    }

    void clear() {
        std::unique_lock lock(mtx);
        clear_internal();
    }

    void insert(size_t pos, const Key& key, const Value& value) {
        std::unique_lock lock(mtx);
        if (pos > count)
            throw std::out_of_range("Index out of range in insert()");
        if (pos == 0) {
            push_front_internal(key, value);
            return;
        }
        if (pos == count) {
            push_back_internal(key, value);
            return;
        }
        auto current = head;
        for (size_t i = 0; i < pos; ++i) {
            current = current->next;
        }
        auto new_node = std::make_shared<Node>(key, value);
        auto prev = current->prev.lock();
        new_node->next = current;
        new_node->prev = prev;
        if (prev) {
            prev->next = new_node;
        }
        current->prev = new_node;
        ++count;
    }

    iterator insert(iterator pos, const Key& key, const Value& value) {
        std::unique_lock lock(mtx);
        if (pos.current == head.get()) {
            push_front_internal(key, value);
            return iterator(head.get(), tail.get());
        }
        auto new_node = std::make_shared<Node>(key, value);
        Node* curr = pos.current;
        Node* prev = curr->prev.lock().get();
        new_node->next = curr;
        new_node->prev = prev;
        if (prev) {
            prev->next = new_node;
        }
        curr->prev = new_node;
        ++count;
        return iterator(new_node.get(), tail.get());
    }

    void erase(size_t pos) {
        std::unique_lock lock(mtx);
        if (pos >= count)
            throw std::out_of_range("Index out of range in erase()");
        if (pos == 0) {
            pop_front_internal();
            return;
        }
        if (pos == count - 1) {
            pop_back_internal();
            return;
        }
        auto current = head;
        for (size_t i = 0; i < pos; ++i) {
            current = current->next;
        }
        auto prev = current->prev.lock();
        auto nxt = current->next;
        if (prev)
            prev->next = nxt;
        if (nxt)
            nxt->prev = prev;
        --count;
    }

    iterator erase(iterator pos) {
        std::unique_lock lock(mtx);
        if (!head) {
            throw std::out_of_range("List is empty");
        }
        if (head.get() == pos.current) {
            pop_front_internal();
            return iterator(head.get(), tail.get());
        }
        if (pos.current == tail.get()) {
            pop_back_internal();
            return iterator(nullptr, tail.get());
        }
        Node* prev = pos.current->prev.lock().get();
        prev->next = pos.current->next;
        if (pos.current->next) {
            pos.current->next->prev = prev;
        }
        --count;
        return iterator(prev->next.get(), tail.get());
    }

    void erase(size_t first, size_t last) {
        std::unique_lock lock(mtx);
        if (first >= count || last > count || first >= last)
            throw std::out_of_range("Invalid range in erase()");
        if (first == 0 && last == count) {
            clear_internal();
            return;
        }
        if (first == 0) {
            auto current = head;
            for (size_t i = 0; i < last; ++i) {
                if (current)
                    current = current->next;
            }
            if (current)
                current->prev.reset();
            head = current;
            count -= (last - first);
            if (count == 0)
                tail.reset();
            return;
        }
        auto prev = head;
        for (size_t i = 0; i < first - 1; ++i) {
            prev = prev->next;
        }
        auto current = prev->next;
        for (size_t i = first; i < last; ++i) {
            if (current)
                current = current->next;
        }
        prev->next = current;
        if (current)
            current->prev = prev;
        else
            tail = prev;
        count -= (last - first);
    }

    // Element access

    std::pair<Key, Value>& front() {
        std::shared_lock lock(mtx);
        if (!head)
            throw std::out_of_range("List is empty");
        return head->data;
    }

    const std::pair<Key, Value>& front() const {
        std::shared_lock lock(mtx);
        if (!head)
            throw std::out_of_range("List is empty");
        return head->data;
    }

    std::pair<Key, Value>& back() {
        std::shared_lock lock(mtx);
        if (!tail)
            throw std::out_of_range("List is empty");
        return tail->data;
    }

    const std::pair<Key, Value>& back() const {
        std::shared_lock lock(mtx);
        if (!tail)
            throw std::out_of_range("List is empty");
        return tail->data;
    }

    std::pair<Key, Value>& at(size_t index) {
        std::shared_lock lock(mtx);
        if (index >= count)
            throw std::out_of_range("Index out of range");
        auto current = head;
        for (size_t i = 0; i < index; ++i)
            current = current->next;
        return current->data;
    }

    const std::pair<Key, Value>& at(size_t index) const {
        std::shared_lock lock(mtx);
        if (index >= count)
            throw std::out_of_range("Index out of range");
        auto current = head;
        for (size_t i = 0; i < index; ++i)
            current = current->next;
        return current->data;
    }

    std::pair<Key, Value>& operator[](size_t index) {
        return at(index);
    }

    const std::pair<Key, Value>& operator[](size_t index) const {
        return at(index);
    }

    // Search methods

    std::optional<std::reference_wrapper<std::pair<Key, Value>>> search(const Key& key) {
        std::shared_lock lock(mtx);
        auto current = head;
        while (current) {
            if (current->data.first == key)
                return std::ref(current->data);
            current = current->next;
        }
        return std::nullopt;
    }

    template<typename Predicate>
    std::optional<std::reference_wrapper<std::pair<Key, Value>>> search_if(Predicate pred) {
        std::shared_lock lock(mtx);
        auto current = head;
        while (current) {
            if (pred(current->data))
                return std::ref(current->data);
            current = current->next;
        }
        return std::nullopt;
    }

    std::optional<size_t> find_index_by_key(const Key& key) const {
        std::shared_lock lock(mtx);
        size_t index = 0;
        auto current = head;
        while (current) {
            if (current->data.first == key)
                return index;
            current = current->next;
            ++index;
        }
        return std::nullopt;
    }

    // Capacity

    bool empty() {
        std::shared_lock lock(mtx);
        return !head;
    }

    size_t size() const {
        std::shared_lock lock(mtx);
        return count;
    }

    //Displaying

    void print() const {
        std::shared_lock lock(mtx); 
        auto current = head;
        while (current) {
            std::cout << "(" << current->data.first << ", " << current->data.second << ") ";
            current = current->next;
        }
        std::cout << std::endl;
    }

    // Floyd’s Cycle Finding Algorithm

    bool cycle() const {
        std::shared_lock lock(mtx);
        auto slow = head, fast = head;
        while (fast && fast->next) {
            slow = slow->next;
            fast = fast->next->next;
            if (slow == fast) {
                return true;
            }
        }
        return false;
    }
};
