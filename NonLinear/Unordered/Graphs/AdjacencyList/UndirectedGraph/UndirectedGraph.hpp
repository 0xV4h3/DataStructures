#pragma once
#include "DoublyLinkedList.hpp"
#include <vector>
#include <stack>
#include <iostream>
#include <atomic>
#include <algorithm> 

template<typename T>
class UndirectedGraph {
public:
    using EdgeList = DoublyLinkedList<size_t, T>;
    using Edge = std::pair<size_t, T>;

    UndirectedGraph() : _vertices(0), _edges(0) {}

    explicit UndirectedGraph(size_t vertices)
        : _vertices(vertices), _edges(0), adjList(vertices) {
    }

    UndirectedGraph(const UndirectedGraph& other) {
        std::shared_lock lock(other.graphMutex);
        _vertices = other._vertices.load();
        _edges = other._edges.load();
        adjList = other.adjList;
    }

    UndirectedGraph(UndirectedGraph&& other) noexcept {
        std::unique_lock lock(other.graphMutex);
        _vertices = other._vertices.load();
        _edges = other._edges.load();
        adjList = std::move(other.adjList);
        other._vertices = 0;
        other._edges = 0;
    }

    UndirectedGraph& operator=(const UndirectedGraph& other) {
        if (this != &other) {
            std::unique_lock lock1(graphMutex, std::defer_lock);
            std::shared_lock lock2(other.graphMutex, std::defer_lock);
            std::lock(lock1, lock2);

            _vertices = other._vertices.load();
            _edges = other._edges.load();
            adjList = other.adjList;
        }
        return *this;
    }

    UndirectedGraph& operator=(UndirectedGraph&& other) noexcept {
        if (this != &other) {
            std::unique_lock lock1(graphMutex, std::defer_lock);
            std::unique_lock lock2(other.graphMutex, std::defer_lock);
            std::lock(lock1, lock2);

            _vertices = other._vertices.load();
            _edges = other._edges.load();
            adjList = std::move(other.adjList);
            other._vertices = 0;
            other._edges = 0;
        }
        return *this;
    }

    ~UndirectedGraph() = default;

    void addVertex() {
        std::unique_lock lock(graphMutex);
        adjList.emplace_back();
        _vertices++;
    }

    void removeVertex(size_t v) {
        std::unique_lock lock(graphMutex);
        if (v >= _vertices)
            throw std::out_of_range("Vertex index out of range in removeVertex()");

        adjList.erase(adjList.begin() + v);

        for (auto& list : adjList) {
            list.erase_if([v](const Edge& edge) {
                return edge.first == v;
                });
        }

        _vertices--;
    }

    void addEdge(size_t u, size_t v, T weight) {
        std::unique_lock lock(graphMutex);
        if (u >= _vertices || v >= _vertices)
            throw std::out_of_range("Vertex index out of range in addEdge()");

        if (!hasEdge_internal(u, v)) {
            adjList[u].push_back(v, weight);
            adjList[v].push_back(u, weight);
            _edges++;
        }
    }

    void removeEdge(size_t u, size_t v) {
        std::unique_lock lock(graphMutex);
        if (u >= _vertices || v >= _vertices)
            throw std::out_of_range("Vertex index out of range in removeEdge()");

        if (adjList[u].search(v).has_value() && adjList[v].search(u).has_value()) {
            adjList[u].erase(v);
            adjList[v].erase(u);
            _edges--;
        }
    }

    bool changeEdge(size_t u, size_t v, T newWeight) {
        std::unique_lock lock(graphMutex);
        if (u >= _vertices || v >= _vertices)
            return false;

        auto resultU = adjList[u].search(v);
        auto resultV = adjList[v].search(u);

        if (resultU.has_value() && resultV.has_value()) {
            resultU->get().second = newWeight;
            resultV->get().second = newWeight;
            return true;
        }
        return false;
    }

    bool hasEdge(size_t u, size_t v) const {
        std::shared_lock lock(graphMutex);
        return hasEdge_internal(u, v);
    }

    T getEdgeWeight(size_t u, size_t v) const {
        std::shared_lock lock(graphMutex);
        if (u >= _vertices || v >= _vertices)
            throw std::out_of_range("Vertex index out of range in getEdgeWeight()");

        auto result = adjList[u].search(v);
        if (result.has_value()) {
            return result->get().second;
        }

        throw std::logic_error("Edge does not exist");
    }

    size_t getVerticesCount() const {
        std::shared_lock lock(graphMutex);
        return _vertices.load();
    }

    size_t getEdgesCount() const {
        std::shared_lock lock(graphMutex);
        return _edges.load();
    }

    std::vector<size_t> getAdjacentVertices(size_t v) const {
        std::shared_lock lock(graphMutex); 
        if (v >= _vertices) {
            throw std::out_of_range("Vertex index out of range in getAdjacentVertices()");
        }

        std::vector<size_t> adjacent;
        for (const auto& edge : adjList[v]) {
            adjacent.push_back(edge.first); 
        }
        return adjacent;
    }

    size_t getAdjacentVerticesCount(size_t v) const {
        std::shared_lock lock(graphMutex); 
        if (v >= _vertices) {
            throw std::out_of_range("Vertex index out of range in getAdjacentVerticesCount()");
        }

        return adjList[v].size(); 
    }

    bool isEmpty() const {
        std::shared_lock lock(graphMutex);
        return _vertices == 0;
    }

    bool hasEdges() const {
        std::shared_lock lock(graphMutex);
        return _edges > 0;
    }

    bool isConnected() const {
        std::shared_lock lock(graphMutex);

        if (_vertices == 0) {
            return true; 
        }

        auto dfs = [this](size_t start, std::vector<bool>& visited) {
            std::stack<size_t> stack;
            stack.push(start);

            while (!stack.empty()) {
                size_t current = stack.top();
                stack.pop();

                if (!visited[current]) {
                    visited[current] = true;
                    for (const auto& edge : adjList[current]) {
                        stack.push(edge.first);
                    }
                }
            }
            };

        std::vector<bool> visited(_vertices, false);
        dfs(0, visited);

        if (std::any_of(visited.begin(), visited.end(), [](bool v) { return !v; })) {
            return false; 
        }

        return true; 
    }

    bool detectCycle() const {
        std::shared_lock lock(graphMutex);
        std::vector<bool> visited(_vertices, false);

        for (size_t i = 0; i < _vertices; ++i) {
            if (!visited[i]) {
                if (detectCycleUtil(i, visited, -1)) {
                    return true;
                }
            }
        }
        return false;
    }

    void printGraph() const {
        std::shared_lock lock(graphMutex);
        for (size_t i = 0; i < adjList.size(); ++i) {
            std::cout << "Vertex " << i << ": ";
            for (const auto& edge : adjList[i]) {
                std::cout << "(" << edge.first << ", " << edge.second << ") ";
            }
            std::cout << "\n";
        }
    }

    // Non-const iterator for the UndirectedGraph
    class iterator {
    public:
        using value_type = Edge;
        using pointer = value_type*;
        using reference = value_type&;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::bidirectional_iterator_tag;

        iterator(typename std::vector<EdgeList>::iterator vertexIt,
            typename std::vector<EdgeList>::iterator vertexEnd,
            typename EdgeList::iterator edgeIt)
            : vertexIt(vertexIt), vertexEnd(vertexEnd), edgeIt(edgeIt) {
            skip_empty_vertex();
        }

        reference operator*() const {
            return *edgeIt;
        }

        pointer operator->() const {
            return &(*edgeIt);
        }

        iterator& operator++() {
            ++edgeIt;
            if (edgeIt == vertexIt->end() && vertexIt != vertexEnd) {
                ++vertexIt;
                skip_empty_vertex();
            }
            return *this;
        }

        iterator operator++(int) {
            iterator temp = *this;
            ++(*this);
            return temp;
        }

        iterator& operator--() {
            if (vertexIt == vertexEnd || edgeIt == vertexIt->begin()) {
                while (vertexIt != vertexEnd && vertexIt->begin() == vertexIt->end()) {
                    --vertexIt;
                }
                edgeIt = vertexIt->end();
            }
            --edgeIt;
            return *this;
        }

        iterator operator--(int) {
            iterator temp = *this;
            --(*this);
            return temp;
        }

        bool operator==(const iterator& other) const {
            return vertexIt == other.vertexIt && edgeIt == other.edgeIt;
        }

        bool operator!=(const iterator& other) const {
            return !(*this == other);
        }

    private:
        void skip_empty_vertex() {
            while (vertexIt != vertexEnd && vertexIt->begin() == vertexIt->end()) {
                ++vertexIt;
            }
            if (vertexIt != vertexEnd) {
                edgeIt = vertexIt->begin();
            }
        }

        typename std::vector<EdgeList>::iterator vertexIt;
        typename std::vector<EdgeList>::iterator vertexEnd;
        typename EdgeList::iterator edgeIt;
    };

    // Const iterator for the UndirectedGraph
    class const_iterator {
    public:
        using value_type = const Edge;
        using pointer = const value_type*;
        using reference = const value_type&;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::bidirectional_iterator_tag;

        const_iterator(typename std::vector<EdgeList>::const_iterator vertexIt,
            typename std::vector<EdgeList>::const_iterator vertexEnd,
            typename EdgeList::const_iterator edgeIt)
            : vertexIt(vertexIt), vertexEnd(vertexEnd), edgeIt(edgeIt) {
            skip_empty_vertex();
        }

        reference operator*() const {
            return *edgeIt;
        }

        pointer operator->() const {
            return &(*edgeIt);
        }

        const_iterator& operator++() {
            ++edgeIt;
            if (edgeIt == vertexIt->end() && vertexIt != vertexEnd) {
                ++vertexIt;
                skip_empty_vertex();
            }
            return *this;
        }

        const_iterator operator++(int) {
            const_iterator temp = *this;
            ++(*this);
            return temp;
        }

        const_iterator& operator--() {
            if (vertexIt == vertexEnd || edgeIt == vertexIt->begin()) {
                while (vertexIt != vertexEnd && vertexIt->begin() == vertexIt->end()) {
                    --vertexIt;
                }
                edgeIt = vertexIt->end();
            }
            --edgeIt;
            return *this;
        }

        const_iterator operator--(int) {
            const_iterator temp = *this;
            --(*this);
            return temp;
        }

        bool operator==(const const_iterator& other) const {
            return vertexIt == other.vertexIt && edgeIt == other.edgeIt;
        }

        bool operator!=(const const_iterator& other) const {
            return !(*this == other);
        }

    private:
        void skip_empty_vertex() {
            while (vertexIt != vertexEnd && vertexIt->begin() == vertexIt->end()) {
                ++vertexIt;
            }
            if (vertexIt != vertexEnd) {
                edgeIt = vertexIt->begin();
            }
        }

        typename std::vector<EdgeList>::const_iterator vertexIt;
        typename std::vector<EdgeList>::const_iterator vertexEnd;
        typename EdgeList::const_iterator edgeIt;
    };

    // Iterator support
    iterator begin() {
        return iterator(adjList.begin(), adjList.end(), adjList.begin()->begin());
    }

    iterator end() {
        return iterator(adjList.end(), adjList.end(), typename EdgeList::iterator());
    }

    const_iterator begin() const {
        return const_iterator(adjList.cbegin(), adjList.cend(), adjList.cbegin()->cbegin());
    }

    const_iterator end() const {
        return const_iterator(adjList.cend(), adjList.cend(), typename EdgeList::const_iterator());
    }

    const_iterator cbegin() const {
        return begin();
    }

    const_iterator cend() const {
        return end();
    }

private:
    bool hasEdge_internal(size_t u, size_t v) const {
        if (u >= _vertices || v >= _vertices)
            return false;

        return adjList[u].search(v).has_value();
    }

    bool detectCycleUtil(size_t v, std::vector<bool>& visited, double parent) const {
        visited[v] = true;

        for (const auto& edge : adjList[v]) {
            size_t adjacent = edge.first;
            if (!visited[adjacent]) {
                if (detectCycleUtil(adjacent, visited, static_cast<double>(v))) {
                    return true;
                }
            }
            else if (static_cast<double>(adjacent) != parent) {
                return true;
            }
        }
        return false;
    }

    std::vector<EdgeList> adjList;
    std::atomic<size_t> _vertices;
    std::atomic<size_t> _edges;
    mutable std::shared_mutex graphMutex;
};