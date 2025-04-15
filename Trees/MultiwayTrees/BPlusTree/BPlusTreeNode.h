#pragma once

#include <vector>
#include <memory>
#include <shared_mutex>
#include <mutex>
#include <utility>
#include <optional>
#include <algorithm>

template<typename Key, typename Value, size_t Degree>
class Node {
private:
    std::vector<Key> _keys;
    std::vector<std::unique_ptr<Node<Key, Value, Degree>>> _children;
    std::vector<Value> _values;
    const bool _isLeaf;
    Node* _parent = nullptr;
    mutable std::shared_mutex _nodeMutex;
    Node* _nextLeaf = nullptr;

public:
    // Constructor
    Node(bool isLeaf) : _isLeaf(isLeaf) {
        _keys.reserve(2 * Degree - 1);
        if (!isLeaf) {
            _children.reserve(2 * Degree);
        }
        else {
            _values.reserve(2 * Degree - 1);
        }
    }

    // Clone method instead of copy constructor
    std::unique_ptr<Node<Key, Value, Degree>> clone() const {
        std::shared_lock<std::shared_mutex> lock(_nodeMutex);
        auto newNode = std::make_unique<Node<Key, Value, Degree>>(_isLeaf);

        newNode->_keys = _keys;
        if (_isLeaf) {
            newNode->_values = _values;
        }

        if (!_isLeaf) {
            for (const auto& child : _children) {
                auto childCopy = child->clone();
                childCopy->_parent = newNode.get();
                newNode->_children.push_back(std::move(childCopy));
            }
        }

        return newNode;
    }

    // Delete copy constructor and assignment to prevent accidental copies
    Node(const Node&) = delete;
    Node& operator=(const Node&) = delete;

    // Move Constructor
    Node(Node&& other) noexcept
        : _keys(std::move(other._keys)),
        _children(std::move(other._children)),
        _values(std::move(other._values)),
        _isLeaf(other._isLeaf),
        _parent(other._parent),
        _nextLeaf(other._nextLeaf)
    {
        if (!_isLeaf) {
            for (auto& child : _children) {
                if (child) {
                    child->_parent = this;
                }
            }
        }

        other._parent = nullptr;
        other._nextLeaf = nullptr;
    }

    // Move Assignment
    Node& operator=(Node&& other) noexcept {
        if (this == &other) return *this;

        std::unique_lock<std::shared_mutex> lock(_nodeMutex);

        if (_isLeaf != other._isLeaf) {
            throw std::logic_error("Cannot move-assign nodes of different types");
        }

        _keys = std::move(other._keys);
        _children = std::move(other._children);
        _values = std::move(other._values);
        _parent = other._parent;
        _nextLeaf = other._nextLeaf;

        if (!_isLeaf) {
            for (auto& child : _children) {
                if (child) {
                    child->_parent = this;
                }
            }
        }

        other._parent = nullptr;
        other._nextLeaf = nullptr;

        return *this;
    }

    // Safe key operations
    void insertKey(const Key& key, size_t index) {
        std::unique_lock<std::shared_mutex> lock(_nodeMutex);
        if (index <= _keys.size()) {
            _keys.insert(_keys.begin() + index, key);
        }
        else {
            throw std::out_of_range("Key insertion index out of range");
        }
    }

    void removeKey(size_t index) {
        std::unique_lock<std::shared_mutex> lock(_nodeMutex);
        if (index < _keys.size()) {
            _keys.erase(_keys.begin() + index);
        }
        else {
            throw std::out_of_range("Key removal index out of range");
        }
    }

    // Safe value operations (only for leaf nodes)
    void insertValue(const Value& value, size_t index) {
        std::unique_lock<std::shared_mutex> lock(_nodeMutex);
        if (!_isLeaf) {
            throw std::logic_error("Cannot insert value in non-leaf node");
        }
        if (index <= _values.size()) {
            _values.insert(_values.begin() + index, value);
        }
        else {
            throw std::out_of_range("Value insertion index out of range");
        }
    }

    void removeValue(size_t index) {
        std::unique_lock<std::shared_mutex> lock(_nodeMutex);
        if (!_isLeaf) {
            throw std::logic_error("Cannot remove value from non-leaf node");
        }
        if (index < _values.size()) {
            _values.erase(_values.begin() + index);
        }
        else {
            throw std::out_of_range("Value removal index out of range");
        }
    }

    // Safe child operations (only for internal nodes)
    void insertChild(std::unique_ptr<Node<Key, Value, Degree>> child, size_t index) {
        std::unique_lock<std::shared_mutex> lock(_nodeMutex);
        if (_isLeaf) {
            throw std::logic_error("Cannot insert child in leaf node");
        }
        if (index <= _children.size()) {
            child->_parent = this;
            _children.insert(_children.begin() + index, std::move(child));
        }
        else {
            throw std::out_of_range("Child insertion index out of range");
        }
    }

    std::unique_ptr<Node<Key, Value, Degree>> removeChild(size_t index) {
        std::unique_lock<std::shared_mutex> lock(_nodeMutex);
        if (_isLeaf) {
            throw std::logic_error("Cannot remove child from leaf node");
        }
        if (index < _children.size()) {
            auto child = std::move(_children[index]);
            _children.erase(_children.begin() + index);
            if (child) {
                child->_parent = nullptr;
            }
            return child;
        }
        else {
            throw std::out_of_range("Child removal index out of range");
        }
    }

    // Find key index in current node
    std::optional<size_t> findKeyIndex(const Key& key) const {
        std::shared_lock<std::shared_mutex> lock(_nodeMutex);
        auto it = std::lower_bound(_keys.begin(), _keys.end(), key);
        if (it != _keys.end() && *it == key) {
            return std::distance(_keys.begin(), it);
        }
        return std::nullopt;
    }

    // Find child index for a given key
    size_t findChildIndex(const Key& key) const {
        std::shared_lock<std::shared_mutex> lock(_nodeMutex);
        if (_isLeaf) {
            throw std::logic_error("Cannot find child index in leaf node");
        }
        auto it = std::upper_bound(_keys.begin(), _keys.end(), key);
        return std::distance(_keys.begin(), it);
    }

    // Safe getters that return copies to prevent data races
    std::vector<Key> getKeysSnapshot() const {
        std::shared_lock<std::shared_mutex> lock(_nodeMutex);
        return _keys;
    }

    std::vector<Value> getValuesSnapshot() const {
        std::shared_lock<std::shared_mutex> lock(_nodeMutex);
        if (!_isLeaf) {
            throw std::logic_error("Cannot get values from non-leaf node");
        }
        return _values;
    }

    // Read-only access to specific elements with bounds checking
    Key getKey(size_t index) const {
        std::shared_lock<std::shared_mutex> lock(_nodeMutex);
        if (index < _keys.size()) {
            return _keys[index];
        }
        throw std::out_of_range("Key index out of range");
    }

    Value getValue(size_t index) const {
        std::shared_lock<std::shared_mutex> lock(_nodeMutex);
        if (!_isLeaf) {
            throw std::logic_error("Cannot get value from non-leaf node");
        }
        if (index < _values.size()) {
            return _values[index];
        }
        throw std::out_of_range("Value index out of range");
    }

    Node<Key, Value, Degree>* getChild(size_t index) const {
        std::shared_lock<std::shared_mutex> lock(_nodeMutex);
        if (_isLeaf) {
            throw std::logic_error("Cannot get child from leaf node");
        }
        if (index < _children.size()) {
            return _children[index].get();
        }
        throw std::out_of_range("Child index out of range");
    }

    // Node type and properties
    bool isLeaf() const noexcept {
        return _isLeaf;
    }

    bool isFull() const noexcept {
        std::shared_lock<std::shared_mutex> lock(_nodeMutex);
        return _keys.size() >= (2 * Degree - 1);
    }

    bool hasMinKeys() const noexcept {
        std::shared_lock<std::shared_mutex> lock(_nodeMutex);
        return _keys.size() >= Degree - 1;
    }

    size_t numKeys() const noexcept {
        std::shared_lock<std::shared_mutex> lock(_nodeMutex);
        return _keys.size();
    }

    size_t numChildren() const noexcept {
        std::shared_lock<std::shared_mutex> lock(_nodeMutex);
        return _children.size();
    }

    // Leaf node chain management
    Node* getNextLeaf() const {
        std::shared_lock<std::shared_mutex> lock(_nodeMutex);
        if (!_isLeaf) {
            return nullptr;  
        }
        return _nextLeaf;
    }

    void setNextLeaf(Node* next) {
        std::unique_lock<std::shared_mutex> lock(_nodeMutex);
        if (!_isLeaf) {
            return;  
        }
        _nextLeaf = next;
    }

    // Parent management
    Node* getParent() const {
        std::shared_lock<std::shared_mutex> lock(_nodeMutex);
        return _parent;
    }

    void setParent(Node* parent) {
        std::unique_lock<std::shared_mutex> lock(_nodeMutex);
        _parent = parent;
    }

    // Node splitting operations
    std::unique_ptr<Node<Key, Value, Degree>> splitLeaf() {
        std::unique_lock<std::shared_mutex> lock(_nodeMutex);
        if (!_isLeaf) {
            throw std::logic_error("Cannot split non-leaf as leaf");
        }

        auto newLeaf = std::make_unique<Node<Key, Value, Degree>>(true);

        size_t midPoint = _keys.size() / 2;

        std::vector<Key> newKeys(_keys.begin() + midPoint, _keys.end());
        _keys.erase(_keys.begin() + midPoint, _keys.end());

        std::vector<Value> newValues(_values.begin() + midPoint, _values.end());
        _values.erase(_values.begin() + midPoint, _values.end());

        newLeaf->_keys = std::move(newKeys);
        newLeaf->_values = std::move(newValues);

        newLeaf->_nextLeaf = _nextLeaf;
        _nextLeaf = newLeaf.get();

        return newLeaf;
    }

    std::unique_ptr<Node<Key, Value, Degree>> splitInternal(Key& midKey) {
        std::unique_lock<std::shared_mutex> lock(_nodeMutex);
        if (_isLeaf) {
            throw std::logic_error("Cannot split leaf as internal");
        }

        auto newNode = std::make_unique<Node<Key, Value, Degree>>(false);

        size_t midPoint = Degree - 1;

        midKey = _keys[midPoint];

        std::vector<Key> newKeys(_keys.begin() + midPoint + 1, _keys.end());
        _keys.erase(_keys.begin() + midPoint, _keys.end());

        std::vector<std::unique_ptr<Node<Key, Value, Degree>>> newChildren;
        for (size_t i = midPoint + 1; i < _children.size(); ++i) {
            newChildren.push_back(std::move(_children[i]));
        }
        _children.resize(midPoint + 1);

        newNode->_keys = std::move(newKeys);

        for (auto& child : newChildren) {
            child->_parent = newNode.get();
        }
        newNode->_children = std::move(newChildren);

        return newNode;
    }
};