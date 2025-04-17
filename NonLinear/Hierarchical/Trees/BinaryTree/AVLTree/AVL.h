#pragma once
#include "BST.h" 

template<typename T>
class AVL : public BST<T> {
protected:
    using Node = typename BST<T>::Node;
    using NodePtr = std::shared_ptr<Node>;

    static int getHeight(const NodePtr& node) {
        return node ? node->_height : 0;
    }

    static void updateHeight(const NodePtr& node) {
        if (node)
            node->_height = std::max(getHeight(node->_leftChild), getHeight(node->_rightChild)) + 1;
    }

    NodePtr rightRotate(const NodePtr& y) {
        NodePtr x = y->_leftChild;
        NodePtr T2 = x->_rightChild;

        x->_rightChild = y;
        y->_leftChild = T2;

        x->_parent = y->_parent;
        y->_parent = x;
        if (T2)
            T2->_parent = y;

        updateHeight(y);
        updateHeight(x);

        return x;
    }

    NodePtr leftRotate(const NodePtr& x) {
        NodePtr y = x->_rightChild;
        NodePtr T2 = y->_leftChild;

        y->_leftChild = x;
        x->_rightChild = T2;

        y->_parent = x->_parent;
        x->_parent = y;
        if (T2)
            T2->_parent = x;

        updateHeight(x);
        updateHeight(y);

        return y;
    }

    NodePtr balanceNode(NodePtr node) {
        updateHeight(node);
        int balance = getHeight(node->_leftChild) - getHeight(node->_rightChild);

        // Left heavy
        if (balance > 1) {
            // LL Case
            if (getHeight(node->_leftChild->_leftChild) >= getHeight(node->_leftChild->_rightChild)) {
                return rightRotate(node);
            }
            // LR Case
            else {
                node->_leftChild = leftRotate(node->_leftChild);
                return rightRotate(node);
            }
        }
        // Right heavy
        else if (balance < -1) {
            // RR Case
            if (getHeight(node->_rightChild->_rightChild) >= getHeight(node->_rightChild->_leftChild)) {
                return leftRotate(node);
            }
            // RL Case
            else {
                node->_rightChild = rightRotate(node->_rightChild);
                return leftRotate(node);
            }
        }
        return node; 
    }
public:
    AVL() : BST<T>() {}
    AVL(const AVL& other) : BST<T>(other) {}
    AVL(AVL&& other) noexcept : BST<T>(std::move(other)) {}
    virtual ~AVL() = default;

    virtual bool Insert(const T& key) override {
        std::unique_lock<std::shared_mutex> lock(this->mtx);
        if (!this->_Root) {
            this->_Root = this->CreateNode(key);
            this->_nodes++;
            return true;
        }

        NodePtr NewNode = this->CreateNode(key);
        NodePtr Current = this->_Root;
        NodePtr Parent = nullptr;
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
        this->_nodes++;

        NodePtr node = Parent;
        while (node) {
            if (node == this->_Root) {
                this->_Root = balanceNode(node);
                this->_Root->_parent.reset();
            }
            else {
                auto parent = node->_parent.lock();
                if (parent->_leftChild == node)
                    parent->_leftChild = balanceNode(node);
                else
                    parent->_rightChild = balanceNode(node);
            }
            node = node->_parent.lock();
        }
        return true;
    }

    virtual bool Delete(const T& key) override {
        std::unique_lock<std::shared_mutex> lock(this->mtx);
        NodePtr TargetNode = std::static_pointer_cast<Node>(this->Search(key));
        if (!TargetNode)
            return false;

        NodePtr Parent = TargetNode->_parent.lock();
        if (BST<T>::isLeaf(TargetNode)) {
            if (TargetNode == this->_Root)
                this->_Root.reset();
            else {
                if (Parent->_leftChild == TargetNode)
                    Parent->_leftChild.reset();
                else
                    Parent->_rightChild.reset();
            }
        }
        else if (!TargetNode->_leftChild || !TargetNode->_rightChild) {
            NodePtr Child = TargetNode->_leftChild ? TargetNode->_leftChild : TargetNode->_rightChild;
            if (TargetNode == this->_Root) {
                this->_Root = Child;
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
            NodePtr SuccessorNode = BST<T>::Minimum_internal(TargetNode->_rightChild);
            TargetNode->_key = SuccessorNode->_key;
            Parent = SuccessorNode->_parent.lock();
            if (Parent->_leftChild == SuccessorNode)
                Parent->_leftChild = SuccessorNode->_rightChild;
            else
                Parent->_rightChild = SuccessorNode->_rightChild;
            if (SuccessorNode->_rightChild)
                SuccessorNode->_rightChild->_parent = Parent;
        }
        this->_nodes--;

        NodePtr node = Parent;
        while (node) {
            if (node == this->_Root) {
                this->_Root = balanceNode(node);
                if (this->_Root)
                    this->_Root->_parent.reset();
            }
            else {
                auto parent = node->_parent.lock();
                if (parent->_leftChild == node)
                    parent->_leftChild = balanceNode(node);
                else
                    parent->_rightChild = balanceNode(node);
            }
            node = node->_parent.lock();
        }
        return true;
    }
};

