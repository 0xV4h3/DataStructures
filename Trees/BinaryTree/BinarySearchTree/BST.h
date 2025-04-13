#pragma once

#include <memory>
#include <vector>
#include <queue>
#include <stack>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <shared_mutex>
#include <optional>
#include <atomic>

template<typename T>
class BST {
protected:
    struct Node {
        T _key;
        int _height = 1;
        std::weak_ptr<Node> _parent;
        std::shared_ptr<Node> _leftChild;
        std::shared_ptr<Node> _rightChild;
        Node(const T& key,
            std::shared_ptr<Node> Parent = nullptr,
            std::shared_ptr<Node> Left = nullptr,
            std::shared_ptr<Node> Right = nullptr)
            : _key(key), _height(1), _parent(Parent), _leftChild(Left), _rightChild(Right) {
        }
    };

    static bool isLeaf(const std::shared_ptr<Node>& node) {
        return (!node->_leftChild && !node->_rightChild);
    }

public:
    BST() : _Root(nullptr), _nodes(0) {}

    BST(const BST& other) : _Root(nullptr), _nodes(0) {
        std::shared_lock<std::shared_mutex> lock(other.mtx);
        if (other._Root) {
            _Root = std::make_shared<Node>(other._Root->_key);
            _nodes++;
            copyTree(_Root, other._Root);
        }
    }

    BST(BST&& other) noexcept : _Root(nullptr), _nodes(0) {
        std::unique_lock<std::shared_mutex> lock(other.mtx);
        _Root = std::move(other._Root);
        _nodes = other._nodes.load();
        other._nodes = 0;
    }

    BST& operator=(const BST& other) {
        if (this != &other) {
            std::unique_lock<std::shared_mutex> lock1(mtx, std::defer_lock);
            std::shared_lock<std::shared_mutex> lock2(other.mtx, std::defer_lock);
            std::lock(lock1, lock2);
            clearTree(_Root);
            _nodes = 0;
            if (other._Root) {
                _Root = std::make_shared<Node>(other._Root->_key);
                _nodes++;
                copyTree(_Root, other._Root);
            }
        }
        return *this;
    }

    BST& operator=(BST&& other) noexcept {
        if (this != &other) {
            std::unique_lock<std::shared_mutex> lock1(mtx, std::defer_lock);
            std::unique_lock<std::shared_mutex> lock2(other.mtx, std::defer_lock);
            std::lock(lock1, lock2);
            clearTree(_Root);
            _Root = std::move(other._Root);
            _nodes = other._nodes.load();
            other._nodes = 0;
        }
        return *this;
    }

    virtual ~BST() {
        std::unique_lock<std::shared_mutex> lock(mtx);
        clearTree(_Root);
        _nodes = 0;
    }

    virtual bool Insert(const T& key) {
        std::unique_lock<std::shared_mutex> lock(mtx);
        if (!_Root) {
            _Root = CreateNode(key);
            _nodes++;
            return true;
        }
        auto NewNode = CreateNode(key);
        auto Current = _Root;
        std::shared_ptr<Node> Parent = nullptr;
        while (Current) {
            Parent = Current;
            if (key < Current->_key)
                Current = Current->_leftChild;
            else if (key > Current->_key)
                Current = Current->_rightChild;
            else
                return false;
        }
        NewNode->_parent = Parent;
        if (key < Parent->_key)
            Parent->_leftChild = NewNode;
        else
            Parent->_rightChild = NewNode;
        _nodes++;
        return true;
    }

    virtual bool Delete(const T& key) {
        std::unique_lock<std::shared_mutex> lock(mtx);
        auto TargetNode = Search(key);
        if (!TargetNode)
            return false;
        if (isLeaf(TargetNode)) {
            if (TargetNode == _Root) {
                _Root.reset();
            }
            else {
                auto Parent = TargetNode->_parent.lock();
                if (Parent->_leftChild == TargetNode)
                    Parent->_leftChild.reset();
                else
                    Parent->_rightChild.reset();
            }
        }
        else if (!TargetNode->_leftChild || !TargetNode->_rightChild) {
            auto Child = TargetNode->_leftChild ? TargetNode->_leftChild : TargetNode->_rightChild;
            auto Parent = TargetNode->_parent.lock();
            if (!Parent) {
                _Root = Child;
                Child->_parent.reset();
            }
            else {
                if (Parent->_leftChild == TargetNode)
                    Parent->_leftChild = Child;
                else
                    Parent->_rightChild = Child;
                Child->_parent = Parent;
            }
        }
        else {
            auto SuccessorNode = Minimum_internal(TargetNode->_rightChild);
            TargetNode->_key = SuccessorNode->_key;
            auto Parent = SuccessorNode->_parent.lock();
            if (Parent->_leftChild == SuccessorNode)
                Parent->_leftChild = SuccessorNode->_rightChild;
            else
                Parent->_rightChild = SuccessorNode->_rightChild;
            if (SuccessorNode->_rightChild)
                SuccessorNode->_rightChild->_parent = Parent;
        }
        _nodes--;
        return true;
    }

    std::shared_ptr<Node> Search(const T& key) const {
        std::shared_lock<std::shared_mutex> lock(mtx);
        auto Current = _Root;
        while (Current) {
            if (Current->_key == key)
                return Current;
            else if (key < Current->_key)
                Current = Current->_leftChild;
            else
                Current = Current->_rightChild;
        }
        return nullptr;
    }

    std::optional<T> MinimumKey() const {
        std::shared_lock<std::shared_mutex> lock(mtx);
        if (!_Root) return std::nullopt; 
        return Minimum_internal(_Root)->_key;
    }

    std::optional<T> MaximumKey() const {
        std::shared_lock<std::shared_mutex> lock(mtx);
        if (!_Root) return std::nullopt;
        return Maximum_internal(_Root)->_key;
    }


    std::shared_ptr<Node> Minimum(std::shared_ptr<Node> node) const {
        std::shared_lock<std::shared_mutex> lock(mtx);
        return Minimum_internal(node);
    }

    std::shared_ptr<Node> Maximum(std::shared_ptr<Node> node) const {
        std::shared_lock<std::shared_mutex> lock(mtx);
        return Maximum_internal(node);
    }

    std::shared_ptr<Node> Successor(std::shared_ptr<Node> node) const {
        std::shared_lock<std::shared_mutex> lock(mtx);
        if (!node) return nullptr;
        if (node->_rightChild)
            return Minimum_internal(node->_rightChild);
        auto Parent = node->_parent.lock();
        while (Parent && node == Parent->_rightChild) {
            node = Parent;
            Parent = Parent->_parent.lock();
        }
        return Parent;
    }

    std::shared_ptr<Node> Predecessor(std::shared_ptr<Node> node) const {
        std::shared_lock<std::shared_mutex> lock(mtx);
        if (!node) return nullptr;
        if (node->_leftChild)
            return Maximum_internal(node->_leftChild);
        auto Parent = node->_parent.lock();
        while (Parent && node == Parent->_leftChild) {
            node = Parent;
            Parent = Parent->_parent.lock();
        }
        return Parent;
    }

    std::shared_ptr<Node> Sibling(std::shared_ptr<Node> node) const {
        std::shared_lock<std::shared_mutex> lock(mtx);
        if (!node) return nullptr;
        auto Parent = node->_parent.lock();
        if (!Parent) return nullptr;
        if (Parent->_leftChild == node)
            return Parent->_rightChild;
        else
            return Parent->_leftChild;
    }

    std::vector<std::shared_ptr<Node>> InorderTraversal() const {
        std::shared_lock<std::shared_mutex> lock(mtx); 
        std::vector<std::shared_ptr<Node>> result;
        InorderTraversal_internal(_Root, result);
        return result;
    }

    std::vector<std::shared_ptr<Node>> PreorderTraversal() const {
        std::shared_lock<std::shared_mutex> lock(mtx); 
        std::vector<std::shared_ptr<Node>> result;
        PreorderTraversal_internal(_Root, result);
        return result;
    }

    std::vector<std::shared_ptr<Node>> PostorderTraversal() const {
        std::shared_lock<std::shared_mutex> lock(mtx); 
        std::vector<std::shared_ptr<Node>> result;
        PostorderTraversal_internal(_Root, result);
        return result;
    }

    std::vector<std::shared_ptr<Node>> LevelOrderTraversal() const {
        std::shared_lock<std::shared_mutex> lock(mtx); 
        std::vector<std::shared_ptr<Node>> result;
        LevelOrderTraversal_internal(_Root, result);
        return result;
    }

    std::vector<std::shared_ptr<Node>> ReverseLevelOrderTraversal() const {
        std::shared_lock<std::shared_mutex> lock(mtx); 
        std::vector<std::shared_ptr<Node>> result;
        ReverseLevelOrderTraversal_internal(_Root, result);
        return result;
    }

    std::vector<std::shared_ptr<Node>> BoundaryTraversal() const {
        std::shared_lock<std::shared_mutex> lock(mtx); 
        std::vector<std::shared_ptr<Node>> result;
        BoundaryTraversal_internal(_Root, result);
        return result;
    }

    std::vector<std::shared_ptr<Node>> DiagonalTraversal() const {
        std::shared_lock<std::shared_mutex> lock(mtx); 
        std::vector<std::shared_ptr<Node>> result;
        DiagonalTraversal_internal(_Root, result);
        return result;
    }

    std::vector<std::shared_ptr<Node>> RangeSearch(const T& low, const T& high) const {
        std::shared_lock<std::shared_mutex> lock(mtx); 
        std::vector<std::shared_ptr<Node>> result;
        RangeSearch_internal(_Root, low, high, result);
        return result;
    }

    std::string toString(const std::vector<std::shared_ptr<Node>>& nodes) const {
        std::stringstream ss;
        for (size_t i = 0; i < nodes.size(); ++i) {
            ss << nodes[i]->_key;
            if (i < nodes.size() - 1)
                ss << ", ";
        }
        return ss.str();
    }

    std::string toStringInorder() const {
        return toString(InorderTraversal());
    }

    std::string toStringPreorder() const {
        return toString(PreorderTraversal());
    }

    std::string toStringPostorder() const {
        return toString(PostorderTraversal());
    }

    std::string toStringLevelOrder() const {
        return toString(LevelOrderTraversal());
    }

    std::string toStringReverseLevelOrder() const {
        return toString(ReverseLevelOrderTraversal());
    }

    std::string toStringBoundary() const {
        return toString(BoundaryTraversal());
    }

    std::string toStringDiagonal() const {
        return toString(DiagonalTraversal());
    }

    std::string toStringRange(const T& low, const T& high) const {
        return toString(RangeSearch(low, high));
    }

    void balance() {
        std::unique_lock<std::shared_mutex> lock(mtx);
        auto nodes = InorderTraversal_internal();
        _Root = buildBalancedTree(nodes, 0, static_cast<int>(nodes.size()) - 1);
    }

    std::shared_ptr<Node> buildBalancedTree(std::vector<std::shared_ptr<Node>>& nodes, int start, int end) {
        if (start > end)
            return nullptr;
        int mid = (start + end) / 2;
        auto node = nodes[mid];
        node->_leftChild = buildBalancedTree(nodes, start, mid - 1);
        node->_rightChild = buildBalancedTree(nodes, mid + 1, end);
        if (node->_leftChild)
            node->_leftChild->_parent = node;
        if (node->_rightChild)
            node->_rightChild->_parent = node;
        return node;
    }

    void visualize() const {
        std::shared_lock<std::shared_mutex> lock(mtx);
        visualize_internal(_Root, "", false);
    }

    std::shared_ptr<Node> getRoot() {
        std::shared_lock<std::shared_mutex> lock(mtx);
        return _Root;
    }

    static std::shared_ptr<Node> getRoot(std::shared_ptr<Node> node) {
        if (!node || node->_parent.expired())
            return node;
        auto Root = node->_parent.lock();
        while (!Root->_parent.expired())
            Root = Root->_parent.lock();
        return Root;
    }

    size_t getNodeCount() const {
        return _nodes.load();
    }

    static int getNodeCount(std::shared_ptr<Node> node) {
        if (!node)
            return 0;
        return 1 + getNodeCount(node->_leftChild) + getNodeCount(node->_rightChild);
    }

    int getDepth() const {
        std::shared_lock<std::shared_mutex> lock(mtx);
        return getDepth_internal(_Root);
    }

    bool isEmpty() const {
        std::shared_lock<std::shared_mutex> lock(mtx);
        return !_Root;
    }

    bool isFull() const {
        std::shared_lock<std::shared_mutex> lock(mtx);
        return isFull_internal(_Root);
    }

    bool isPerfect() const {
        std::shared_lock<std::shared_mutex> lock(mtx);
        return isPerfect_internal(_Root, getDepth_internal(_Root), 0);
    }

    bool isBalanced() const {
        std::shared_lock<std::shared_mutex> lock(mtx);
        return isBalanced_internal(_Root) != -1;
    }

    bool isComplete() const {
        std::shared_lock<std::shared_mutex> lock(mtx);
        return isComplete_internal(_Root);
    }

    bool isDegenerate() const {
        std::shared_lock<std::shared_mutex> lock(mtx);
        return isDegenerate_internal(_Root);
    }

protected:
    std::shared_ptr<Node> CreateNode(const T& key) const {
        return std::make_shared<Node>(key);
    }

    static void addLeftBoundary(std::shared_ptr<Node> node, std::vector<std::shared_ptr<Node>>& result) {
        auto current = node->_leftChild;
        while (current) {
            if (!isLeaf(current))
                result.push_back(current);
            current = current->_leftChild ? current->_leftChild : current->_rightChild;
        }
    }

    static void addLeaves(std::shared_ptr<Node> node, std::vector<std::shared_ptr<Node>>& result) {
        if (!node)
            return;
        if (isLeaf(node))
            result.push_back(node);
        addLeaves(node->_leftChild, result);
        addLeaves(node->_rightChild, result);
    }

    static void addRightBoundary(std::shared_ptr<Node> node, std::vector<std::shared_ptr<Node>>& result) {
        std::stack<std::shared_ptr<Node>> stack;
        auto current = node->_rightChild;
        while (current) {
            if (!isLeaf(current))
                stack.push(current);
            current = current->_rightChild ? current->_rightChild : current->_leftChild;
        }
        while (!stack.empty()) {
            result.push_back(stack.top());
            stack.pop();
        }
    }

protected:
    std::shared_ptr<Node> _Root;
    std::atomic<size_t> _nodes;
    mutable std::shared_mutex mtx;

    void copyTree(std::shared_ptr<Node> dest, std::shared_ptr<Node> source) {
        if (!source) return;
        if (source->_leftChild) {
            dest->_leftChild = std::make_shared<Node>(source->_leftChild->_key, dest);
            _nodes++;
            copyTree(dest->_leftChild, source->_leftChild);
        }
        if (source->_rightChild) {
            dest->_rightChild = std::make_shared<Node>(source->_rightChild->_key, dest);
            _nodes++;
            copyTree(dest->_rightChild, source->_rightChild);
        }
    }

    void clearTree(std::shared_ptr<Node>& node) {
        if (!node) return;
        clearTree(node->_leftChild);
        clearTree(node->_rightChild);
        node.reset();
    }

    // Internal versions (non thread-safe)
    static std::shared_ptr<Node> Minimum_internal(std::shared_ptr<Node> node) {
        if (!node) return nullptr;
        auto Min = node;
        while (Min->_leftChild)
            Min = Min->_leftChild;
        return Min;
    }

    static std::shared_ptr<Node> Maximum_internal(std::shared_ptr<Node> node) {
        if (!node) return nullptr;
        auto Max = node;
        while (Max->_rightChild)
            Max = Max->_rightChild;
        return Max;
    }

    std::vector<std::shared_ptr<Node>> InorderTraversal_internal() const {
        std::vector<std::shared_ptr<Node>> result;
        InorderTraversal_internal(_Root, result);
        return result;
    }

    static void InorderTraversal_internal(std::shared_ptr<Node> node, std::vector<std::shared_ptr<Node>>& result) {
        if (node) {
            InorderTraversal_internal(node->_leftChild, result);
            result.push_back(node);
            InorderTraversal_internal(node->_rightChild, result);
        }
    }

    static void PreorderTraversal_internal(std::shared_ptr<Node> node, std::vector<std::shared_ptr<Node>>& result) {
        if (node) {
            result.push_back(node);
            PreorderTraversal_internal(node->_leftChild, result);
            PreorderTraversal_internal(node->_rightChild, result);
        }
    }

    static void PostorderTraversal_internal(std::shared_ptr<Node> node, std::vector<std::shared_ptr<Node>>& result) {
        if (node) {
            PostorderTraversal_internal(node->_leftChild, result);
            PostorderTraversal_internal(node->_rightChild, result);
            result.push_back(node);
        }
    }

    static void LevelOrderTraversal_internal(std::shared_ptr<Node> node, std::vector<std::shared_ptr<Node>>& result) {
        if (!node) return;
        std::queue<std::shared_ptr<Node>> queue;
        queue.push(node);
        while (!queue.empty()) {
            auto current = queue.front();
            queue.pop();
            result.push_back(current);
            if (current->_leftChild)
                queue.push(current->_leftChild);
            if (current->_rightChild)
                queue.push(current->_rightChild);
        }
    }

    static void ReverseLevelOrderTraversal_internal(std::shared_ptr<Node> node, std::vector<std::shared_ptr<Node>>& result) {
        if (!node) return;
        std::queue<std::shared_ptr<Node>> queue;
        std::stack<std::shared_ptr<Node>> stack;
        queue.push(node);
        while (!queue.empty()) {
            auto current = queue.front();
            queue.pop();
            stack.push(current);
            if (current->_rightChild)
                queue.push(current->_rightChild);
            if (current->_leftChild)
                queue.push(current->_leftChild);
        }
        while (!stack.empty()) {
            result.push_back(stack.top());
            stack.pop();
        }
    }

    static void BoundaryTraversal_internal(std::shared_ptr<Node> node, std::vector<std::shared_ptr<Node>>& result) {
        if (!node) return;
        if (!isLeaf(node))
            result.push_back(node);
        addLeftBoundary(node, result);
        addLeaves(node, result);
        addRightBoundary(node, result);
    }

    static void DiagonalTraversal_internal(std::shared_ptr<Node> node, std::vector<std::shared_ptr<Node>>& result) {
        if (!node) return;
        std::queue<std::shared_ptr<Node>> queue;
        queue.push(node);
        while (!queue.empty()) {
            auto current = queue.front();
            queue.pop();
            while (current) {
                result.push_back(current);
                if (current->_leftChild)
                    queue.push(current->_leftChild);
                current = current->_rightChild;
            }
        }
    }

    static void RangeSearch_internal(std::shared_ptr<Node> node, const T& low, const T& high, std::vector<std::shared_ptr<Node>>& result) {
        if (!node) return;
        if (node->_key > low)
            RangeSearch_internal(node->_leftChild, low, high, result);
        if (node->_key >= low && node->_key <= high)
            result.push_back(node);
        if (node->_key < high)
            RangeSearch_internal(node->_rightChild, low, high, result);
    }

    static void visualize_internal(std::shared_ptr<Node> node, std::string prefix, bool isLeft) {
        if (node) {
            std::cout << prefix;
            std::cout << (isLeft ? "|-- " : "\\-- ");
            std::cout << node->_key << std::endl;
            visualize_internal(node->_leftChild, prefix + (isLeft ? "|   " : "    "), true);
            visualize_internal(node->_rightChild, prefix + (isLeft ? "|   " : "    "), false);
        }
    }

    static int getDepth_internal(std::shared_ptr<Node> node) {
        if (!node)
            return 0;
        int leftDepth = getDepth_internal(node->_leftChild);
        int rightDepth = getDepth_internal(node->_rightChild);
        return 1 + std::max(leftDepth, rightDepth);
    }

    static bool isFull_internal(std::shared_ptr<Node> node) {
        if (!node)
            return true;
        if (isLeaf(node))
            return true;
        if (node->_leftChild && node->_rightChild)
            return isFull_internal(node->_leftChild) && isFull_internal(node->_rightChild);
        return false;
    }

    static bool isPerfect_internal(std::shared_ptr<Node> node, int depth, int level) {
        if (!node)
            return true;
        if (isLeaf(node))
            return (level == depth - 1);
        if (!node->_leftChild || !node->_rightChild)
            return false;
        return isPerfect_internal(node->_leftChild, depth, level + 1) && isPerfect_internal(node->_rightChild, depth, level + 1);
    }

    static int isBalanced_internal(std::shared_ptr<Node> node) {
        if (!node)
            return 0;
        int leftHeight = isBalanced_internal(node->_leftChild);
        if (leftHeight == -1) return -1;
        int rightHeight = isBalanced_internal(node->_rightChild);
        if (rightHeight == -1) return -1;
        if (std::abs(leftHeight - rightHeight) > 1)
            return -1;
        return std::max(leftHeight, rightHeight) + 1;
    }

    static bool isComplete_internal(std::shared_ptr<Node> node) {
        if (!node)
            return true;
        std::queue<std::shared_ptr<Node>> queue;
        queue.push(node);
        bool reachedEnd = false;
        while (!queue.empty()) {
            auto current = queue.front();
            queue.pop();
            if (!current)
                reachedEnd = true;
            else {
                if (reachedEnd)
                    return false;
                queue.push(current->_leftChild);
                queue.push(current->_rightChild);
            }
        }
        return true;
    }

    static bool isDegenerate_internal(std::shared_ptr<Node> node) {
        if (!node)
            return true;
        if (node->_leftChild && node->_rightChild)
            return false;
        return isDegenerate_internal(node->_leftChild) || isDegenerate_internal(node->_rightChild);
    }
};