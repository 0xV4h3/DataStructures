#pragma once

#include "BPlusTreeNode.hpp"
#include <memory>
#include <optional>
#include <shared_mutex>
#include <vector>
#include <utility>
#include <functional>
#include <iostream>
#include <algorithm>

template<typename Key, typename Value, size_t Degree = 4>
class BPlusTree {
private:
    std::unique_ptr<Node<Key, Value, Degree>> _root;
    mutable std::shared_mutex _treeMutex;
    size_t _size = 0;

public:
    // Constructor
    BPlusTree() : _root(std::make_unique<Node<Key, Value, Degree>>(true)) {}

    // Destructor - handled automatically by unique_ptr
    ~BPlusTree() = default;

    // Copy constructor - performs deep copy
    BPlusTree(const BPlusTree& other) {
        std::shared_lock<std::shared_mutex> lock(other._treeMutex);
        _root = other._root->clone();
        _size = other._size;
    }

    // Move constructor
    BPlusTree(BPlusTree&& other) noexcept {
        std::unique_lock<std::shared_mutex> lock(other._treeMutex);
        _root = std::move(other._root);
        _size = other._size;
        other._size = 0;
        other._root = std::make_unique<Node<Key, Value, Degree>>(true);
    }

    // Copy assignment operator
    BPlusTree& operator=(const BPlusTree& other) {
        if (this != &other) {
            BPlusTree temp(other);
            std::swap(_root, temp._root);
            _size = temp._size;
        }
        return *this;
    }

    // Move assignment operator
    BPlusTree& operator=(BPlusTree&& other) noexcept {
        if (this != &other) {
            std::unique_lock<std::shared_mutex> lockThis(_treeMutex);
            std::unique_lock<std::shared_mutex> lockOther(other._treeMutex);
            _root = std::move(other._root);
            _size = other._size;
            other._size = 0;
            other._root = std::make_unique<Node<Key, Value, Degree>>(true);
        }
        return *this;
    }

    // Insert a key-value pair
    bool insert(const Key& key, const Value& value) {
        std::unique_lock<std::shared_mutex> lock(_treeMutex);

        if (_root->isFull()) {
            auto newRoot = std::make_unique<Node<Key, Value, Degree>>(false);
            auto oldRoot = std::move(_root);
            _root = std::move(newRoot);
            _root->insertChild(std::move(oldRoot), 0);
            splitChild(_root.get(), 0);
        }

        bool result = insertNonFull(_root.get(), key, value);
        if (result) {
            _size++;
        }
        return result;
    }

    // Find a value by key
    std::optional<Value> find(const Key& key) const {
        std::shared_lock<std::shared_mutex> lock(_treeMutex);
        Node<Key, Value, Degree>* current = _root.get();

        while (!current->isLeaf()) {
            size_t index = current->findChildIndex(key);
            current = current->getChild(index);
        }

        auto keyIndex = current->findKeyIndex(key);
        if (keyIndex.has_value()) {
            return current->getValue(keyIndex.value());
        }

        return std::nullopt;
    }

    // Remove a key-value pair
    bool remove(const Key& key) {
        std::unique_lock<std::shared_mutex> lock(_treeMutex);
        bool result = removeKey(_root.get(), key);

        if (!_root->isLeaf() && _root->numKeys() == 0) {
            auto newRoot = _root->removeChild(0);
            _root = std::move(newRoot);
        }

        if (result) {
            _size--;
        }

        return result;
    }

    // Get the number of key-value pairs
    size_t size() const noexcept {
        std::shared_lock<std::shared_mutex> lock(_treeMutex);
        return _size;
    }

    // Check if the tree is empty
    bool empty() const noexcept {
        std::shared_lock<std::shared_mutex> lock(_treeMutex);
        return _size == 0;
    }

    // Clear the tree
    void clear() {
        std::unique_lock<std::shared_mutex> lock(_treeMutex);
        _root = std::make_unique<Node<Key, Value, Degree>>(true);
        _size = 0;
    }

    // Traverse the tree in order and apply a function to each key-value pair
    void traverse(const std::function<void(const Key&, const Value&)>& func) const {
        std::shared_lock<std::shared_mutex> lock(_treeMutex);
        traverseInOrder(_root.get(), func);
    }

    // Range query: find all key-value pairs in [start, end]
    std::vector<std::pair<Key, Value>> rangeQuery(const Key& start, const Key& end) const {
        std::shared_lock<std::shared_mutex> lock(_treeMutex);
        std::vector<std::pair<Key, Value>> result;

        Node<Key, Value, Degree>* leaf = findLeaf(_root.get(), start);

        while (leaf != nullptr) {
            auto keys = leaf->getKeysSnapshot();
            auto values = leaf->getValuesSnapshot();

            for (size_t i = 0; i < keys.size(); ++i) {
                if (keys[i] >= start && keys[i] <= end) {
                    result.emplace_back(keys[i], values[i]);
                }

                if (keys[i] > end) {
                    return result;
                }
            }

            leaf = leaf->getNextLeaf();
        }

        return result;
    }

    // Debug method to print tree structure
    void printTree(std::ostream& os = std::cout) const {
        std::shared_lock<std::shared_mutex> lock(_treeMutex);
        os << "B+ Tree (Degree " << Degree << ", Size " << _size << "):" << std::endl;
        printNode(_root.get(), 0, os);
    }

private:
    // Helper method to split a child node
    void splitChild(Node<Key, Value, Degree>* parent, size_t childIndex) {
        Node<Key, Value, Degree>* child = parent->getChild(childIndex);

        if (child->isLeaf()) {
            auto newChild = child->splitLeaf();

            parent->insertKey(newChild->getKey(0), childIndex);

            parent->insertChild(std::move(newChild), childIndex + 1);
        }
        else {
            Key midKey;
            auto newChild = child->splitInternal(midKey);

            parent->insertKey(midKey, childIndex);

            parent->insertChild(std::move(newChild), childIndex + 1);
        }
    }

    // Helper method to insert into a non-full node
    bool insertNonFull(Node<Key, Value, Degree>* node, const Key& key, const Value& value) {
        if (node->isLeaf()) {
            auto keyIndex = node->findKeyIndex(key);
            if (keyIndex.has_value()) {
                return false;  
            }

            auto keys = node->getKeysSnapshot();
            size_t i = 0;
            while (i < keys.size() && keys[i] < key) {
                i++;
            }

            node->insertKey(key, i);
            node->insertValue(value, i);
            return true;
        }
        else {
            auto keys = node->getKeysSnapshot();
            size_t i = 0;
            while (i < keys.size() && keys[i] <= key) {
                i++;
            }

            Node<Key, Value, Degree>* child = node->getChild(i);

            if (child->isFull()) {
                splitChild(node, i);

                if (node->getKey(i) < key) {
                    child = node->getChild(i + 1);
                }
                else {
                    child = node->getChild(i);
                }
            }

            return insertNonFull(child, key, value);
        }
    }

    // Helper method to traverse the tree in order
    void traverseInOrder(Node<Key, Value, Degree>* node,
        const std::function<void(const Key&, const Value&)>& func) const {
        if (node->isLeaf()) {
            auto keys = node->getKeysSnapshot();
            auto values = node->getValuesSnapshot();

            for (size_t i = 0; i < keys.size(); ++i) {
                func(keys[i], values[i]);
            }
        }
        else {
            auto keys = node->getKeysSnapshot();

            traverseInOrder(node->getChild(0), func);

            for (size_t i = 0; i < keys.size(); ++i) {
                traverseInOrder(node->getChild(i + 1), func);
            }
        }
    }

    // Helper method to find a leaf node that may contain the key
    Node<Key, Value, Degree>* findLeaf(Node<Key, Value, Degree>* node, const Key& key) const {
        if (node->isLeaf()) {
            return node;
        }

        size_t index = node->findChildIndex(key);
        return findLeaf(node->getChild(index), key);
    }

    // Helper method to remove a key
    bool removeKey(Node<Key, Value, Degree>* node, const Key& key) {
        if (node->isLeaf()) {
            // Find the key
            auto keyIndex = node->findKeyIndex(key);
            if (!keyIndex.has_value()) {
                return false;  
            }

            node->removeKey(keyIndex.value());
            node->removeValue(keyIndex.value());

            return true;
        }
        else {
            size_t index = node->findChildIndex(key);
            Node<Key, Value, Degree>* child = node->getChild(index);

            if (!child->hasMinKeys()) {
                if (index > 0) {
                    Node<Key, Value, Degree>* leftSibling = node->getChild(index - 1);
                    if (leftSibling->numKeys() > Degree - 1) {
                        borrowFromLeft(node, index);
                    }
                    else if (index < node->numChildren() - 1) {
                        Node<Key, Value, Degree>* rightSibling = node->getChild(index + 1);
                        if (rightSibling->numKeys() > Degree - 1) {
                            borrowFromRight(node, index);
                        }
                        else {
                            if (index > 0) {
                                mergeNodes(node, index - 1);  
                                index = index - 1;  
                            }
                            else {
                                mergeNodes(node, index);  
                            }
                            child = node->getChild(index);
                        }
                    }
                    else {
                        mergeNodes(node, index - 1);
                        index = index - 1;
                        child = node->getChild(index);
                    }
                }
                else if (index < node->numChildren() - 1) {
                    mergeNodes(node, index);
                    child = node->getChild(index);
                }
            }

            return removeKey(child, key);
        }
    }

    // Helper method to borrow a key from left sibling
    void borrowFromLeft(Node<Key, Value, Degree>* parent, size_t childIndex) {
        Node<Key, Value, Degree>* child = parent->getChild(childIndex);
        Node<Key, Value, Degree>* leftSibling = parent->getChild(childIndex - 1);

        if (!child->isLeaf()) {
            Key parentKey = parent->getKey(childIndex - 1);
            child->insertKey(parentKey, 0);

            Key leftLastKey = leftSibling->getKey(leftSibling->numKeys() - 1);
            parent->removeKey(childIndex - 1);
            parent->insertKey(leftLastKey, childIndex - 1);

            if (leftSibling->numChildren() > 0) {
                auto lastChild = leftSibling->removeChild(leftSibling->numChildren() - 1);
                child->insertChild(std::move(lastChild), 0);
            }

            leftSibling->removeKey(leftSibling->numKeys() - 1);
        }
        else {
            Key leftLastKey = leftSibling->getKey(leftSibling->numKeys() - 1);
            Value leftLastValue = leftSibling->getValue(leftSibling->numKeys() - 1);

            child->insertKey(leftLastKey, 0);
            child->insertValue(leftLastValue, 0);

            parent->removeKey(childIndex - 1);
            parent->insertKey(leftLastKey, childIndex - 1);

            leftSibling->removeKey(leftSibling->numKeys() - 1);
            leftSibling->removeValue(leftSibling->numKeys() - 1);
        }
    }

    // Helper method to borrow a key from right sibling
    void borrowFromRight(Node<Key, Value, Degree>* parent, size_t childIndex) {
        Node<Key, Value, Degree>* child = parent->getChild(childIndex);
        Node<Key, Value, Degree>* rightSibling = parent->getChild(childIndex + 1);

        if (!child->isLeaf()) {
            Key parentKey = parent->getKey(childIndex);
            child->insertKey(parentKey, child->numKeys());

            Key rightFirstKey = rightSibling->getKey(0);
            parent->removeKey(childIndex);
            parent->insertKey(rightFirstKey, childIndex);

            if (rightSibling->numChildren() > 0) {
                auto firstChild = rightSibling->removeChild(0);
                child->insertChild(std::move(firstChild), child->numChildren());
            }

            rightSibling->removeKey(0);
        }
        else {
            Key rightFirstKey = rightSibling->getKey(0);
            Value rightFirstValue = rightSibling->getValue(0);

            child->insertKey(rightFirstKey, child->numKeys());
            child->insertValue(rightFirstValue, child->numKeys());

            parent->removeKey(childIndex);

            Key newSeparator = rightSibling->getKey(1);
            parent->insertKey(newSeparator, childIndex);

            rightSibling->removeKey(0);
            rightSibling->removeValue(0);
        }
    }

    // Helper method to merge two nodes
    void mergeNodes(Node<Key, Value, Degree>* parent, size_t leftChildIndex) {
        Node<Key, Value, Degree>* leftChild = parent->getChild(leftChildIndex);
        Node<Key, Value, Degree>* rightChild = parent->getChild(leftChildIndex + 1);

        if (!leftChild->isLeaf()) {
            Key separator = parent->getKey(leftChildIndex);
            leftChild->insertKey(separator, leftChild->numKeys());

            auto rightKeys = rightChild->getKeysSnapshot();
            for (size_t i = 0; i < rightKeys.size(); ++i) {
                leftChild->insertKey(rightKeys[i], leftChild->numKeys());
            }

            for (size_t i = 0; i < rightChild->numChildren(); ++i) {
                auto child = rightChild->removeChild(i);
                leftChild->insertChild(std::move(child), leftChild->numChildren());
            }
        }
        else {
            auto rightKeys = rightChild->getKeysSnapshot();
            auto rightValues = rightChild->getValuesSnapshot();

            for (size_t i = 0; i < rightKeys.size(); ++i) {
                leftChild->insertKey(rightKeys[i], leftChild->numKeys());
                leftChild->insertValue(rightValues[i], leftChild->numKeys());
            }

            leftChild->setNextLeaf(rightChild->getNextLeaf());
        }

        parent->removeKey(leftChildIndex);
        parent->removeChild(leftChildIndex + 1);
    }

    // Helper method to print tree structure (for debugging)
    void printNode(Node<Key, Value, Degree>* node, int level, std::ostream& os) const {
        if (node == nullptr) return;

        for (int i = 0; i < level; ++i) {
            os << "  ";
        }

        auto keys = node->getKeysSnapshot();
        os << "Node [" << (node->isLeaf() ? "Leaf" : "Internal") << "]: ";

        for (size_t i = 0; i < keys.size(); ++i) {
            os << keys[i];
            if (i < keys.size() - 1) {
                os << ", ";
            }
        }
        os << std::endl;

        if (!node->isLeaf()) {
            for (size_t i = 0; i < node->numChildren(); ++i) {
                printNode(node->getChild(i), level + 1, os);
            }
        }
    }
};