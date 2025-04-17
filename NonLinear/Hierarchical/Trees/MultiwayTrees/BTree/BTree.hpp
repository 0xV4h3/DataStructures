#pragma once

#include "BTreeNode.hpp"
#include <functional>
#include <iostream>

template<typename Key, typename Value, size_t Degree = 3>
class BTree {
private:
    std::unique_ptr<BTreeNode<Key, Value, Degree>> _root;
    mutable std::shared_mutex _treeMutex;
    size_t _size = 0;

public:
    // Constructor
    BTree() : _root(std::make_unique<BTreeNode<Key, Value, Degree>>(true)) {}

    // Destructor 
    ~BTree() = default;

    // Copy constructor - performs deep copy
    BTree(const BTree& other) {
        std::shared_lock<std::shared_mutex> lock(other._treeMutex);
        _root = other._root->clone();
        _size = other._size;
    }

    // Move constructor
    BTree(BTree&& other) noexcept {
        std::unique_lock<std::shared_mutex> lock(other._treeMutex);
        _root = std::move(other._root);
        _size = other._size;
        other._size = 0;
        other._root = std::make_unique<BTreeNode<Key, Value, Degree>>(true);
    }

    // Copy assignment operator
    BTree& operator=(const BTree& other) {
        if (this != &other) {
            BTree temp(other);
            std::swap(_root, temp._root);
            _size = temp._size;
        }
        return *this;
    }

    // Move assignment operator
    BTree& operator=(BTree&& other) noexcept {
        if (this != &other) {
            std::unique_lock<std::shared_mutex> lockThis(_treeMutex);
            std::unique_lock<std::shared_mutex> lockOther(other._treeMutex);
            _root = std::move(other._root);
            _size = other._size;
            other._size = 0;
            other._root = std::make_unique<BTreeNode<Key, Value, Degree>>(true);
        }
        return *this;
    }

    // Insert a key-value pair
    bool insert(const Key& key, const Value& value) {
        std::unique_lock<std::shared_mutex> lock(_treeMutex);

        if (_root->isFull()) {
            auto newRoot = std::make_unique<BTreeNode<Key, Value, Degree>>(false);
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
        return findValue(_root.get(), key);
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
        _root = std::make_unique<BTreeNode<Key, Value, Degree>>(true);
        _size = 0;
    }

    // Traverse the tree in-order and apply a function to each key-value pair
    void traverse(const std::function<void(const Key&, const Value&)>& func) const {
        std::shared_lock<std::shared_mutex> lock(_treeMutex);
        traverseInOrder(_root.get(), func);
    }

    // Debug method to print tree structure
    void printTree(std::ostream& os = std::cout) const {
        std::shared_lock<std::shared_mutex> lock(_treeMutex);
        os << "B-Tree (Degree " << Degree << ", Size " << _size << "):" << std::endl;
        printNode(_root.get(), 0, os);
    }

private:
    // Helper method to find value in a node and its subtree
    std::optional<Value> findValue(BTreeNode<Key, Value, Degree>* node, const Key& key) const {
        if (!node) return std::nullopt;

        auto keyIndex = node->findKeyIndex(key);
        if (keyIndex.has_value()) {
            return node->getValue(keyIndex.value());
        }

        if (node->isLeaf()) {
            return std::nullopt;
        }

        size_t childIndex = node->findChildIndex(key);
        return findValue(node->getChild(childIndex), key);
    }

    // Helper method to split a child node
    void splitChild(BTreeNode<Key, Value, Degree>* parent, size_t childIndex) {
        BTreeNode<Key, Value, Degree>* child = parent->getChild(childIndex);

        auto [midKey, newChild] = child->split();

        parent->insertKey(midKey, childIndex);

        if (child->isLeaf()) {
            auto keyIndex = child->findKeyIndex(midKey);
            if (keyIndex.has_value()) {
                parent->insertValue(child->getValue(keyIndex.value()), childIndex);
            }
        }

        parent->insertChild(std::move(newChild), childIndex + 1);
    }

    // Helper method to insert into a non-full node
    bool insertNonFull(BTreeNode<Key, Value, Degree>* node, const Key& key, const Value& value) {
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
            size_t i = node->findChildIndex(key);

            BTreeNode<Key, Value, Degree>* child = node->getChild(i);

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
    void traverseInOrder(BTreeNode<Key, Value, Degree>* node,
        const std::function<void(const Key&, const Value&)>& func) const {
        if (!node) return;

        if (node->isLeaf()) {
            auto keys = node->getKeysSnapshot();
            auto values = node->getValuesSnapshot();

            for (size_t i = 0; i < keys.size(); ++i) {
                func(keys[i], values[i]);
            }
        }
        else {
            auto keys = node->getKeysSnapshot();
            auto values = node->getValuesSnapshot();

            if (node->numChildren() > 0) {
                traverseInOrder(node->getChild(0), func);
            }

            for (size_t i = 0; i < keys.size(); ++i) {
                func(keys[i], values[i]);

                if (i + 1 < node->numChildren()) {
                    traverseInOrder(node->getChild(i + 1), func);
                }
            }
        }
    }

    // Helper methods for remove operation
    bool removeKey(BTreeNode<Key, Value, Degree>* node, const Key& key) {
        if (!node) return false;

        auto keyIndex = node->findKeyIndex(key);

        if (keyIndex.has_value()) {
            size_t index = keyIndex.value();

            if (node->isLeaf()) {
                node->removeKey(index);
                node->removeValue(index);
                return true;
            }
            else {
                BTreeNode<Key, Value, Degree>* leftChild = node->getChild(index);
                BTreeNode<Key, Value, Degree>* rightChild = node->getChild(index + 1);

                if (leftChild->numKeys() >= Degree) {
                    auto [predKey, predValue] = findPredecessor(leftChild);
                    node->removeKey(index);
                    node->insertKey(predKey, index);
                    node->removeValue(index);
                    node->insertValue(predValue, index);
                    return removeKey(leftChild, predKey);
                }
                else if (rightChild->numKeys() >= Degree) {
                    auto [succKey, succValue] = findSuccessor(rightChild);
                    node->removeKey(index);
                    node->insertKey(succKey, index);
                    node->removeValue(index);
                    node->insertValue(succValue, index);
                    return removeKey(rightChild, succKey);
                }
                else {
                    mergeNodes(node, index);
                    return removeKey(leftChild, key);
                }
            }
        }
        else {
            if (node->isLeaf()) {
                return false;
            }

            size_t childIndex = node->findChildIndex(key);
            BTreeNode<Key, Value, Degree>* child = node->getChild(childIndex);

            if (child->numKeys() < Degree) {
                ensureMinKeys(node, childIndex);
                child = node->getChild(childIndex);
            }

            return removeKey(child, key);
        }
    }

    // Find the predecessor (rightmost key in left subtree)
    std::pair<Key, Value> findPredecessor(BTreeNode<Key, Value, Degree>* node) {
        while (!node->isLeaf()) {
            node = node->getChild(node->numChildren() - 1);
        }

        size_t lastIndex = node->numKeys() - 1;
        return { node->getKey(lastIndex), node->getValue(lastIndex) };
    }

    // Find the successor (leftmost key in right subtree)
    std::pair<Key, Value> findSuccessor(BTreeNode<Key, Value, Degree>* node) {
        while (!node->isLeaf()) {
            node = node->getChild(0);
        }

        return { node->getKey(0), node->getValue(0) };
    }

    // Ensure a child has at least t keys
    void ensureMinKeys(BTreeNode<Key, Value, Degree>* parent, size_t childIndex) {
        BTreeNode<Key, Value, Degree>* child = parent->getChild(childIndex);

        if (childIndex > 0) {
            BTreeNode<Key, Value, Degree>* leftSibling = parent->getChild(childIndex - 1);

            if (leftSibling->numKeys() >= Degree) {
                borrowFromLeft(parent, childIndex);
                return;
            }
        }

        if (childIndex < parent->numChildren() - 1) {
            BTreeNode<Key, Value, Degree>* rightSibling = parent->getChild(childIndex + 1);

            if (rightSibling->numKeys() >= Degree) {
                borrowFromRight(parent, childIndex);
                return;
            }
        }

        if (childIndex > 0) {
            mergeNodes(parent, childIndex - 1);
        }
        else {
            mergeNodes(parent, childIndex);
        }
    }

    // Borrow a key from left sibling
    void borrowFromLeft(BTreeNode<Key, Value, Degree>* parent, size_t childIndex) {
        BTreeNode<Key, Value, Degree>* child = parent->getChild(childIndex);
        BTreeNode<Key, Value, Degree>* leftSibling = parent->getChild(childIndex - 1);

        child->insertKey(parent->getKey(childIndex - 1), 0);
        child->insertValue(parent->getValue(childIndex - 1), 0);

        Key lastKey = leftSibling->getKey(leftSibling->numKeys() - 1);
        Value lastValue = leftSibling->getValue(leftSibling->numKeys() - 1);
        parent->removeKey(childIndex - 1);
        parent->insertKey(lastKey, childIndex - 1);
        parent->removeValue(childIndex - 1);
        parent->insertValue(lastValue, childIndex - 1);

        if (!leftSibling->isLeaf()) {
            auto rightmostChild = leftSibling->removeChild(leftSibling->numChildren() - 1);
            child->insertChild(std::move(rightmostChild), 0);
        }

        leftSibling->removeKey(leftSibling->numKeys() - 1);
        leftSibling->removeValue(leftSibling->numValues() - 1);
    }

    // Borrow a key from right sibling
    void borrowFromRight(BTreeNode<Key, Value, Degree>* parent, size_t childIndex) {
        BTreeNode<Key, Value, Degree>* child = parent->getChild(childIndex);
        BTreeNode<Key, Value, Degree>* rightSibling = parent->getChild(childIndex + 1);

        child->insertKey(parent->getKey(childIndex), child->numKeys());
        child->insertValue(parent->getValue(childIndex), child->numValues());

        Key firstKey = rightSibling->getKey(0);
        Value firstValue = rightSibling->getValue(0);
        parent->removeKey(childIndex);
        parent->insertKey(firstKey, childIndex);
        parent->removeValue(childIndex);
        parent->insertValue(firstValue, childIndex);

        if (!rightSibling->isLeaf()) {
            auto leftmostChild = rightSibling->removeChild(0);
            child->insertChild(std::move(leftmostChild), child->numChildren());
        }

        rightSibling->removeKey(0);
        rightSibling->removeValue(0);
    }

    // Merge node[childIndex+1] into node[childIndex]
    void mergeNodes(BTreeNode<Key, Value, Degree>* parent, size_t leftChildIndex) {
        BTreeNode<Key, Value, Degree>* leftChild = parent->getChild(leftChildIndex);
        BTreeNode<Key, Value, Degree>* rightChild = parent->getChild(leftChildIndex + 1);

        Key parentKey = parent->getKey(leftChildIndex);
        Value parentValue = parent->getValue(leftChildIndex);
        leftChild->insertKey(parentKey, leftChild->numKeys());
        leftChild->insertValue(parentValue, leftChild->numValues());

        auto rightKeys = rightChild->getKeysSnapshot();
        auto rightValues = rightChild->getValuesSnapshot();

        for (size_t i = 0; i < rightKeys.size(); ++i) {
            leftChild->insertKey(rightKeys[i], leftChild->numKeys());
            leftChild->insertValue(rightValues[i], leftChild->numValues());
        }

        if (!leftChild->isLeaf()) {
            for (size_t i = 0; i < rightChild->numChildren(); ++i) {
                auto child = rightChild->removeChild(i);
                leftChild->insertChild(std::move(child), leftChild->numChildren());
            }
        }

        parent->removeKey(leftChildIndex);
        parent->removeValue(leftChildIndex);
        parent->removeChild(leftChildIndex + 1);
    }

    // Print a node and its children recursively
    void printNode(BTreeNode<Key, Value, Degree>* node, int level, std::ostream& os) const {
        if (!node) return;

        std::string indent(level * 4, ' ');

        os << indent << "Node [";
        auto keys = node->getKeysSnapshot();

        for (size_t i = 0; i < keys.size(); ++i) {
            if (i > 0) os << ", ";
            os << keys[i];
        }
        os << "] (";

        if (node->isLeaf()) {
            os << "leaf";
        }
        else {
            os << "internal";
        }
        os << ")" << std::endl;

        if (!node->isLeaf()) {
            for (size_t i = 0; i < node->numChildren(); ++i) {
                os << indent << "Child " << i << ":" << std::endl;
                printNode(node->getChild(i), level + 1, os);
            }
        }
    }
};