#pragma once
#include <optional>
#include <memory>
#include <shared_mutex>
#include <mutex>
#include <utility>
#include <stdexcept>
#include <functional>

template<typename Key, typename Value>
class SinglyLinkedList {
private:
    struct Node {
        std::pair<Key, Value> data;
        std::shared_ptr<Node> next;
        Node(const Key& key, const Value& value) : data(key, value), next(nullptr) {}
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
        head = new_node;
        if (!tail) {
            tail = new_node;
        }
        ++count;
    }

    void push_back_internal(const Key& key, const Value& value) {
        auto new_node = std::make_shared<Node>(key, value);
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
        if (!head) {
            tail.reset();
        }
        --count;
        return result;
    }

    std::optional<std::pair<Key, Value>> pop_back_internal() {
        if (!head)
            return std::nullopt;
        if (head == tail) {
            auto result = head->data;
            head.reset();
            tail.reset();
            --count;
            return result;
        }
        auto current = head;
        while (current->next != tail) {
            current = current->next;
        }
        auto result = tail->data;
        tail = current;
        tail->next.reset();
        --count;
        return result;
    }

public:
    // Iterator for non-const access
    class iterator {
        friend class SinglyLinkedList;
    public:
        using value_type = std::pair<Key, Value>;
        using pointer = value_type*;
        using reference = value_type&;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::forward_iterator_tag;

        iterator(Node* ptr) : current(ptr) {}

        reference operator*() const { return current->data; }
        pointer operator->() const { return &(current->data); }

        iterator& operator++() {
            current = current->next.get();
            return *this;
        }
        iterator operator++(int) {
            iterator temp = *this;
            ++(*this);
            return temp;
        }
        bool operator==(const iterator& other) const { return current == other.current; }
        bool operator!=(const iterator& other) const { return current != other.current; }
    private:
        Node* current;
    };

    // Iterator for const access
    class const_iterator {
        friend class SinglyLinkedList;
    public:
        using value_type = const std::pair<Key, Value>;
        using pointer = const value_type*;
        using reference = const value_type&;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::forward_iterator_tag;

        const_iterator(const Node* ptr) : current(ptr) {}

        reference operator*() const { return current->data; }
        pointer operator->() const { return &(current->data); }

        const_iterator& operator++() {
            current = current->next.get();
            return *this;
        }
        const_iterator operator++(int) {
            const_iterator temp = *this;
            ++(*this);
            return temp;
        }
        bool operator==(const const_iterator& other) const { return current == other.current; }
        bool operator!=(const const_iterator& other) const { return current != other.current; }
    private:
        const Node* current;
    };

    // begin/end functions for iterator support
    iterator begin() {
        return iterator(head.get());
    }
    iterator end() {
        return iterator(nullptr);
    }
    const_iterator begin() const {
        return const_iterator(head.get());
    }
    const_iterator end() const {
        return const_iterator(nullptr);
    }
    const_iterator cbegin() const {
        return const_iterator(head.get());
    }
    const_iterator cend() const {
        return const_iterator(nullptr);
    }

    // Default constructor
    SinglyLinkedList() : head(nullptr), tail(nullptr), count(0) {}

    // Copy constructor (deep copy)
    SinglyLinkedList(const SinglyLinkedList& other) {
        std::shared_lock lock(other.mtx);
        clear_internal();
        auto current = other.head;
        while (current) {
            push_back(current->data.first, current->data.second);
            current = current->next;
        }
    }

    // Move constructor
    SinglyLinkedList(SinglyLinkedList&& other) noexcept {
        std::unique_lock lock(other.mtx);
        head = std::move(other.head);
        tail = std::move(other.tail);
        count = other.count;
        other.count = 0;
    }

    // Copy assignment operator
    SinglyLinkedList& operator=(const SinglyLinkedList& other) {
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
    SinglyLinkedList& operator=(SinglyLinkedList&& other) noexcept {
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
    ~SinglyLinkedList() {
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
        for (size_t i = 0; i < pos - 1; ++i) {
            current = current->next;
        }
        auto new_node = std::make_shared<Node>(key, value);
        new_node->next = current->next;
        current->next = new_node;
        ++count;
    }

    iterator insert(iterator pos, const Key& key, const Value& value) {
        std::unique_lock lock(mtx);
        if (pos.current == head.get()) {
            push_front_internal(key, value);
            return iterator(head.get());
        }
        Node* prev = head.get();
        while (prev && prev->next.get() != pos.current) {
            prev = prev->next.get();
        }
        if (!prev) {
            throw std::out_of_range("Invalid iterator in insert()");
        }
        auto new_node = std::make_shared<Node>(key, value);
        new_node->next = prev->next;
        prev->next = new_node;
        ++count;
        return iterator(new_node.get());
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
        for (size_t i = 0; i < pos - 1; ++i) {
            current = current->next;
        }
        auto node_to_remove = current->next;
        current->next = node_to_remove->next;
        if (node_to_remove == tail) {
            tail = current;
        }
        --count;
    }

    iterator erase(iterator pos) {
        std::unique_lock lock(mtx);
        if (!head) {
            throw std::out_of_range("List is empty");
        }
        if (head.get() == pos.current) {
            pop_front_internal();
            return iterator(head.get());
        }
        if (pos.current == tail.get()) {
            pop_back_internal();
            return iterator(nullptr);
        }
        Node* prev = head.get();
        while (prev && prev->next.get() != pos.current) {
            prev = prev->next.get();
        }
        if (!prev) {
            throw std::out_of_range("Invalid iterator in erase()");
        }
        prev->next = pos.current->next;
        --count;
        return iterator(prev->next.get());
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
            head = current;
            if (!head)
                tail.reset();
            count -= (last - first);
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
        if (!current)
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
        std::shared_lock lock(mtx);
        if (index >= count) {
            throw std::out_of_range("Index out of range");
        }
        auto current = head;
        for (size_t i = 0; i < index; ++i) {
            current = current->next;
        }
        return current->data;
    }

    const std::pair<Key, Value>& operator[](size_t index) const {
        std::shared_lock lock(mtx);
        if (index >= count) {
            throw std::out_of_range("Index out of range");
        }
        auto current = head;
        for (size_t i = 0; i < index; ++i) {
            current = current->next;
        }
        return current->data;
    }

    // Search methods

    std::optional<std::reference_wrapper<std::pair<Key, Value>>> search(const Key& key) {
        std::shared_lock lock(mtx);
        auto current = head;
        while (current) {
            if (current->data.first == key)
                return current->data;
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
                return current->data;
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

    //Capacity

    bool empty() {
        std::shared_lock lock(mtx);
        return !head;
    }

    size_t size() const {
        std::shared_lock lock(mtx);
        return count;
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
