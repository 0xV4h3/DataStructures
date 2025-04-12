#pragma once
#include "ConcurrentHashMap.h"

template<typename Key, typename Value>
class LRU {
private:
    struct Node {
        std::pair<Key, Value> data;
        std::shared_ptr<Node> next;
        std::weak_ptr<Node> prev;

        Node(const Key& key, const Value& value) : data(key, value) {}
    };

    std::shared_ptr<Node> head;
    std::shared_ptr<Node> tail;
    mutable std::shared_mutex mtx;

    static constexpr size_t MINIMAL_CAPACITY = 4;
    static constexpr size_t DEFAULT_CAPACITY = 100;
    size_t capacity;

    ConcurrentHashMap<Key, std::shared_ptr<Node>> cacheMap;

    void setCapacity(size_t capacity_) {
        if (capacity_ < MINIMAL_CAPACITY) {
            throw std::invalid_argument("Capacity must be at least " + std::to_string(MINIMAL_CAPACITY));
        }
        capacity = capacity_;
    }

    void add(std::shared_ptr<Node> node) {
        node->next = head->next;
        node->prev = head;
        head->next->prev = node;
        head->next = node;
    }

    void remove(std::shared_ptr<Node> node) {
        node->prev.lock()->next = node->next;
        node->next->prev = node->prev;
    }

public:
    explicit LRU(size_t capacity_ = DEFAULT_CAPACITY) : capacity(capacity_), cacheMap(capacity_) {
        head = std::make_shared<Node>(Key{}, Value{});
        tail = std::make_shared<Node>(Key{}, Value{});
        head->next = tail;
        tail->prev = head;
    }

    std::optional<Value> get(const Key& key) {
        std::unique_lock lock(mtx);
        auto nodeOpt = cacheMap.search(key);
        if (!nodeOpt) return std::nullopt;

        std::shared_ptr<Node> node = nodeOpt.value();
        remove(node);
        add(node);
        return node->data.second;
    }

    void put(const Key& key, const Value& value) {
        std::unique_lock lock(mtx);
        auto nodeOpt = cacheMap.search(key);

        if (nodeOpt) {
            std::shared_ptr<Node> node = nodeOpt.value();
            remove(node);
            node->data.second = value;
            add(node);
        }
        else {
            auto node = std::make_shared<Node>(key, value);
            cacheMap.insert(key, node);
            add(node);

            if (cacheMap.getElementsCount() > capacity) {
                std::shared_ptr<Node> nodeToDelete = tail->prev.lock();
                remove(nodeToDelete);
                cacheMap.remove(nodeToDelete->data.first);
            }
        }
    }
};
