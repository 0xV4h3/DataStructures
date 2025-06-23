#pragma once
#include "BST.hpp" 
#include <memory>
#include <iostream>
#include <algorithm>

template<typename T>
class RBT : public BST<T> {
public:
    enum Color { RED, BLACK };

    struct RBTNode : public BST<T>::Node {
        Color _color;
        RBTNode(const T& key, Color color = RED)
            : BST<T>::Node(key), _color(color) {}
    };

    using Node = RBTNode;
    using NodePtr = std::shared_ptr<Node>;

    // Override CreateNode to construct RBTNode
    std::shared_ptr<Node> CreateNode(const T& key, Color color = RED) const {
        return std::make_shared<Node>(key, color);
    }

    void leftRotate(NodePtr x) {
        NodePtr y = std::static_pointer_cast<Node>(x->_rightChild);
        x->_rightChild = y->_leftChild;
        if (y->_leftChild)
            y->_leftChild->_parent = x;
        y->_parent = x->_parent;
        if (auto xparent = x->_parent.lock()) {
            if (xparent->_leftChild == x)
                xparent->_leftChild = y;
            else
                xparent->_rightChild = y;
        }
        else {
            this->_Root = y;
        }
        y->_leftChild = x;
        x->_parent = y;
    }

    void rightRotate(NodePtr y) {
        NodePtr x = std::static_pointer_cast<Node>(y->_leftChild);
        y->_leftChild = x->_rightChild;
        if (x->_rightChild)
            x->_rightChild->_parent = y;
        x->_parent = y->_parent;
        if (auto yparent = y->_parent.lock()) {
            if (yparent->_leftChild == y)
                yparent->_leftChild = x;
            else
                yparent->_rightChild = x;
        }
        else {
            this->_Root = x;
        }
        x->_rightChild = y;
        y->_parent = x;
    }

    void insertFixUp(NodePtr z) {
        while (z->_parent.lock() && std::static_pointer_cast<Node>(z->_parent.lock())->_color == RED) {
            auto parent = std::static_pointer_cast<Node>(z->_parent.lock());
            auto grandparent = parent->_parent.lock() ? std::static_pointer_cast<Node>(parent->_parent.lock()) : nullptr;
            if (!grandparent) break;

            if (parent == grandparent->_leftChild) {
                auto y = std::static_pointer_cast<Node>(grandparent->_rightChild);
                if (y && y->_color == RED) {
                    // Case 1
                    parent->_color = BLACK;
                    y->_color = BLACK;
                    grandparent->_color = RED;
                    z = grandparent;
                }
                else {
                    if (z == parent->_rightChild) {
                        // Case 2
                        z = parent;
                        leftRotate(z);
                        parent = std::static_pointer_cast<Node>(z->_parent.lock());
                        grandparent = parent && parent->_parent.lock() ? std::static_pointer_cast<Node>(parent->_parent.lock()) : nullptr;
                    }
                    // Case 3
                    parent->_color = BLACK;
                    if (grandparent) {
                        grandparent->_color = RED;
                        rightRotate(grandparent);
                    }
                }
            }
            else {
                auto y = std::static_pointer_cast<Node>(grandparent->_leftChild);
                if (y && y->_color == RED) {
                    // Case 1
                    parent->_color = BLACK;
                    y->_color = BLACK;
                    grandparent->_color = RED;
                    z = grandparent;
                }
                else {
                    if (z == parent->_leftChild) {
                        // Case 2
                        z = parent;
                        rightRotate(z);
                        parent = std::static_pointer_cast<Node>(z->_parent.lock());
                        grandparent = parent && parent->_parent.lock() ? std::static_pointer_cast<Node>(parent->_parent.lock()) : nullptr;
                    }
                    // Case 3
                    parent->_color = BLACK;
                    if (grandparent) {
                        grandparent->_color = RED;
                        leftRotate(grandparent);
                    }
                }
            }
        }
        std::static_pointer_cast<Node>(this->_Root)->_color = BLACK;
    }

    void transplant(NodePtr u, NodePtr v) {
        if (!u->_parent.lock()) {
            this->_Root = v;
        }
        else if (u == std::static_pointer_cast<Node>(u->_parent.lock())->_leftChild) {
            std::static_pointer_cast<Node>(u->_parent.lock())->_leftChild = v;
        }
        else {
            std::static_pointer_cast<Node>(u->_parent.lock())->_rightChild = v;
        }
        if (v) v->_parent = u->_parent;
    }

    void deleteFixUp(NodePtr x, NodePtr xParent) {
        while (x != this->_Root && (!x || x->_color == BLACK)) {
            if (x == std::static_pointer_cast<Node>(xParent->_leftChild)) {
                NodePtr w = std::static_pointer_cast<Node>(xParent->_rightChild);
                if (w && w->_color == RED) {
                    // Case 1
                    w->_color = BLACK;
                    xParent->_color = RED;
                    leftRotate(xParent);
                    w = std::static_pointer_cast<Node>(xParent->_rightChild);
                }
                if ((!w->_leftChild || std::static_pointer_cast<Node>(w->_leftChild)->_color == BLACK) &&
                    (!w->_rightChild || std::static_pointer_cast<Node>(w->_rightChild)->_color == BLACK)) {
                    // Case 2
                    w->_color = RED;
                    x = xParent;
                    xParent = x->_parent.lock() ? std::static_pointer_cast<Node>(x->_parent.lock()) : nullptr;
                }
                else {
                    if (!w->_rightChild || std::static_pointer_cast<Node>(w->_rightChild)->_color == BLACK) {
                        // Case 3
                        if (w->_leftChild) std::static_pointer_cast<Node>(w->_leftChild)->_color = BLACK;
                        w->_color = RED;
                        rightRotate(w);
                        w = std::static_pointer_cast<Node>(xParent->_rightChild);
                    }
                    // Case 4
                    w->_color = std::static_pointer_cast<Node>(xParent)->_color;
                    xParent->_color = BLACK;
                    if (w->_rightChild) std::static_pointer_cast<Node>(w->_rightChild)->_color = BLACK;
                    leftRotate(xParent);
                    x = std::static_pointer_cast<Node>(this->_Root);
                }
            }
            else {
                NodePtr w = std::static_pointer_cast<Node>(xParent->_leftChild);
                if (w && w->_color == RED) {
                    // Case 1
                    w->_color = BLACK;
                    xParent->_color = RED;
                    rightRotate(xParent);
                    w = std::static_pointer_cast<Node>(xParent->_leftChild);
                }
                if ((!w->_leftChild || std::static_pointer_cast<Node>(w->_leftChild)->_color == BLACK) &&
                    (!w->_rightChild || std::static_pointer_cast<Node>(w->_rightChild)->_color == BLACK)) {
                    // Case 2
                    w->_color = RED;
                    x = xParent;
                    xParent = x->_parent.lock() ? std::static_pointer_cast<Node>(x->_parent.lock()) : nullptr;
                }
                else {
                    if (!w->_leftChild || std::static_pointer_cast<Node>(w->_leftChild)->_color == BLACK) {
                        // Case 3
                        if (w->_rightChild) std::static_pointer_cast<Node>(w->_rightChild)->_color = BLACK;
                        w->_color = RED;
                        leftRotate(w);
                        w = std::static_pointer_cast<Node>(xParent->_leftChild);
                    }
                    // Case 4
                    w->_color = std::static_pointer_cast<Node>(xParent)->_color;
                    xParent->_color = BLACK;
                    if (w->_leftChild) std::static_pointer_cast<Node>(w->_leftChild)->_color = BLACK;
                    rightRotate(xParent);
                    x = std::static_pointer_cast<Node>(this->_Root);
                }
            }
        }
        if (x) x->_color = BLACK;
    }

public:
    RBT() : BST<T>() {}
    RBT(const RBT& other) : BST<T>(other) {}
    RBT(RBT&& other) noexcept : BST<T>(std::move(other)) {}
    virtual ~RBT() = default;

    virtual bool Insert(const T& key) override {
        std::unique_lock<std::shared_mutex> lock(this->mtx);
        NodePtr z = CreateNode(key, RED);
        NodePtr y = nullptr;
        NodePtr x = std::static_pointer_cast<Node>(this->_Root);

        while (x) {
            y = x;
            if (key < x->_key)
                x = std::static_pointer_cast<Node>(x->_leftChild);
            else if (key > x->_key)
                x = std::static_pointer_cast<Node>(x->_rightChild);
            else
                return false;
        }

        z->_parent = y;
        if (!y) {
            this->_Root = z;
        }
        else if (key < y->_key) {
            y->_leftChild = z;
        }
        else {
            y->_rightChild = z;
        }
        this->_nodes++;
        insertFixUp(z);
        return true;
    }

    virtual bool Delete(const T& key) override {
        std::unique_lock<std::shared_mutex> lock(this->mtx);
        NodePtr z = std::static_pointer_cast<Node>(this->Search_internal(key));
        if (!z) return false;

        NodePtr y = z;
        NodePtr x = nullptr;
        Color yOriginalColor = y->_color;
        NodePtr xParent = nullptr;

        if (!z->_leftChild) {
            x = std::static_pointer_cast<Node>(z->_rightChild);
            xParent = z->_parent.lock() ? std::static_pointer_cast<Node>(z->_parent.lock()) : nullptr;
            transplant(z, x);
        }
        else if (!z->_rightChild) {
            x = std::static_pointer_cast<Node>(z->_leftChild);
            xParent = z->_parent.lock() ? std::static_pointer_cast<Node>(z->_parent.lock()) : nullptr;
            transplant(z, x);
        }
        else {
            y = std::static_pointer_cast<Node>(BST<T>::Minimum_internal(z->_rightChild));
            yOriginalColor = y->_color;
            x = std::static_pointer_cast<Node>(y->_rightChild);
            if (y->_parent.lock() == z) {
                if (x) x->_parent = y;
                xParent = y;
            }
            else {
                transplant(y, x);
                y->_rightChild = z->_rightChild;
                if (y->_rightChild)
                    y->_rightChild->_parent = y;
                xParent = y->_parent.lock() ? std::static_pointer_cast<Node>(y->_parent.lock()) : nullptr;
            }
            transplant(z, y);
            y->_leftChild = z->_leftChild;
            if (y->_leftChild)
                y->_leftChild->_parent = y;
            y->_color = z->_color;
        }

        this->_nodes--;
        if (yOriginalColor == BLACK) {
            // If x is null, we must pass its parent for fix-up
            if (x)
                deleteFixUp(x, x->_parent.lock() ? std::static_pointer_cast<Node>(x->_parent.lock()) : nullptr);
            else if (xParent)
                deleteFixUp(x, xParent);
        }
        return true;
    }

    void visualize() const override {
        std::shared_lock<std::shared_mutex> lock(this->mtx);
        visualize_internal(std::static_pointer_cast<Node>(this->_Root), "", false);
    }

    void visualize_internal(std::shared_ptr<typename BST<T>::Node> node, std::string prefix, bool isLeft) const override {
        if (node) {
            std::cout << prefix;
            std::cout << (isLeft ? "|-- " : "\\-- ");
            auto rbtNode = std::static_pointer_cast<RBTNode>(node);
            std::cout << rbtNode->_key << (rbtNode->_color == RED ? " [R]" : " [B]") << std::endl;
            visualize_internal(node->_leftChild, prefix + (isLeft ? "|   " : "    "), true);
            visualize_internal(node->_rightChild, prefix + (isLeft ? "|   " : "    "), false);
        }
    }
};