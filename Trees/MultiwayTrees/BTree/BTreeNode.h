#pragma once

#include <vector>
#include <memory>
#include <shared_mutex>
#include <mutex>
#include <utility>
#include <optional>
#include <algorithm>
#include <stdexcept>

template<typename Key, typename Value, size_t Degree>
class BTreeNode {
private:
    std::vector<Key> _keys;
    std::vector<std::unique_ptr<BTreeNode<Key, Value, Degree>>> _children;
    std::vector<Value> _values;
    const bool _isLeaf;
    BTreeNode* _parent = nullptr;
    mutable std::shared_mutex _nodeMutex;

public:
    // Constructor
    BTreeNode(bool isLeaf) : _isLeaf(isLeaf) {
        _keys.reserve(2 * Degree - 1);
        if (!isLeaf) {
            _children.reserve(2 * Degree);
        }
        else {
            _values.reserve(2 * Degree - 1);
        }
    }

    // Clone method instead of copy constructor
    std::unique_ptr<BTreeNode<Key, Value, Degree>> clone() const {
        std::shared_lock<std::shared_mutex> lock(_nodeMutex);
        auto newNode = std::make_unique<BTreeNode<Key, Value, Degree>>(_isLeaf);

        newNode->_keys = _keys;
        newNode->_values = _values;

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
    BTreeNode(const BTreeNode&) = delete;
    BTreeNode& operator=(const BTreeNode&) = delete;

    // Move Constructor
    BTreeNode(BTreeNode&& other) noexcept
        : _keys(std::move(other._keys)),
        _children(std::move(other._children)),
        _values(std::move(other._values)),
        _isLeaf(other._isLeaf),
        _parent(other._parent)
    {

        if (!_isLeaf) {
            for (auto& child : _children) {
                if (child) {
                    child->_parent = this;
                }
            }
        }

        other._parent = nullptr;
    }

    // Move Assignment
    BTreeNode& operator=(BTreeNode&& other) noexcept {
        if (this == &other) return *this;

        std::unique_lock<std::shared_mutex> lock(_nodeMutex);

        if (_isLeaf != other._isLeaf) {
            throw std::logic_error("Cannot move-assign nodes of different types");
        }

        _keys = std::move(other._keys);
        _children = std::move(other._children);
        _values = std::move(other._values);
        _parent = other._parent;

        if (!_isLeaf) {
            for (auto& child : _children) {
                if (child) {
                    child->_parent = this;
                }
            }
        }

        other._parent = nullptr;

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

    // Safe value operations
    void insertValue(const Value& value, size_t index) {
        std::unique_lock<std::shared_mutex> lock(_nodeMutex);
        if (index <= _values.size()) {
            _values.insert(_values.begin() + index, value);
        }
        else {
            throw std::out_of_range("Value insertion index out of range");
        }
    }

    void removeValue(size_t index) {
        std::unique_lock<std::shared_mutex> lock(_nodeMutex);
        if (index < _values.size()) {
            _values.erase(_values.begin() + index);
        }
        else {
            throw std::out_of_range("Value removal index out of range");
        }
    }

    // Safe child operations (only for internal nodes)
    void insertChild(std::unique_ptr<BTreeNode<Key, Value, Degree>> child, size_t index) {
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

    std::unique_ptr<BTreeNode<Key, Value, Degree>> removeChild(size_t index) {
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
        if (index < _values.size()) {
            return _values[index];
        }
        throw std::out_of_range("Value index out of range");
    }

    BTreeNode<Key, Value, Degree>* getChild(size_t index) const {
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

    size_t numValues() const noexcept {
        std::shared_lock<std::shared_mutex> lock(_nodeMutex);
        return _values.size();
    }

    size_t numChildren() const noexcept {
        std::shared_lock<std::shared_mutex> lock(_nodeMutex);
        return _children.size();
    }

    // Parent management
    BTreeNode* getParent() const {
        std::shared_lock<std::shared_mutex> lock(_nodeMutex);
        return _parent;
    }

    void setParent(BTreeNode* parent) {
        std::unique_lock<std::shared_mutex> lock(_nodeMutex);
        _parent = parent;
    }

    // Node splitting operations
    std::pair<Key, std::unique_ptr<BTreeNode<Key, Value, Degree>>> split() {
        std::unique_lock<std::shared_mutex> lock(_nodeMutex);

        size_t midPoint = _keys.size() / 2;

        Key midKey = _keys[midPoint];

        auto newNode = std::make_unique<BTreeNode<Key, Value, Degree>>(_isLeaf);

        for (size_t i = midPoint + 1; i < _keys.size(); ++i) {
            newNode->_keys.push_back(_keys[i]);
        }
        _keys.resize(midPoint);

        if (_isLeaf) {
            for (size_t i = midPoint + 1; i < _values.size(); ++i) {
                newNode->_values.push_back(_values[i]);
            }
            _values.resize(midPoint);

            newNode->_keys.insert(newNode->_keys.begin(), midKey);
            if (midPoint < _values.size()) {
                newNode->_values.insert(newNode->_values.begin(), _values[midPoint]);
                _keys.pop_back();
                _values.pop_back();
            }
        }
        else {
            for (size_t i = midPoint + 1; i <= _children.size(); ++i) {
                if (i < _children.size()) {
                    auto child = std::move(_children[i]);
                    child->_parent = newNode.get();
                    newNode->_children.push_back(std::move(child));
                }
            }
            _children.resize(midPoint + 1);
        }

        return { midKey, std::move(newNode) };
    }
};