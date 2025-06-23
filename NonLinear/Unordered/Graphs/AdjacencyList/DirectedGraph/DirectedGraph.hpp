#pragma once
#include "DoublyLinkedList.hpp"
#include <vector>
#include <iostream>
#include <atomic>

template<typename T>
class DirectedGraph {
public:
    using EdgeList = DoublyLinkedList<size_t, T>;
    using Edge = std::pair<size_t, T>;

    DirectedGraph() : _vertices(0), _edges(0) {}

    explicit DirectedGraph(size_t vertices)
        : _vertices(vertices), _edges(0), adjList(vertices) {
    }

    DirectedGraph(const DirectedGraph& other) {
        std::shared_lock lock(other.graphMutex);
        _vertices = other._vertices.load();
        _edges = other._edges.load();
        adjList = other.adjList;
    }

    DirectedGraph(DirectedGraph&& other) noexcept {
        std::unique_lock lock(other.graphMutex);
        _vertices = other._vertices.load();
        _edges = other._edges.load();
        adjList = std::move(other.adjList);
        other._vertices = 0;
        other._edges = 0;
    }

    DirectedGraph& operator=(const DirectedGraph& other) {
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

    DirectedGraph& operator=(DirectedGraph&& other) noexcept {
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

    ~DirectedGraph() = default;

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
            _edges++;
        }
    }

    void removeEdge(size_t u, size_t v) {
        std::unique_lock lock(graphMutex);
        if (u >= _vertices || v >= _vertices)
            throw std::out_of_range("Vertex index out of range in removeEdge()");

        if (adjList[u].search(v).has_value()) {
            adjList[u].erase(v);
            _edges--;
        }
    }

    bool changeEdge(size_t u, size_t v, T newWeight) {
        std::unique_lock lock(graphMutex);
        if (u >= _vertices || v >= _vertices)
            return false;

        auto result = adjList[u].search(v);
        if (result.has_value()) {
            result->get().second = newWeight;
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

    size_t getOutgoingEdgesCount(size_t v) const {
        std::shared_lock lock(graphMutex);
        if (v >= _vertices)
            throw std::out_of_range("Vertex index out of range in getOutgoingEdgesCount()");

        return adjList[v].size();
    }

    size_t getIncomingEdgesCount(size_t v) const {
        std::shared_lock lock(graphMutex);
        if (v >= _vertices)
            throw std::out_of_range("Vertex index out of range in getIncomingEdgesCount()");

        size_t count = 0;
        for (size_t i = 0; i < _vertices; ++i) {
            if (hasEdge_internal(i, v))
                count++;
        }
        return count;
    }

    std::vector<size_t> getOutgoingEdges(size_t v) const {
        std::shared_lock lock(graphMutex);
        if (v >= _vertices)
            throw std::out_of_range("Vertex index out of range in getOutgoingEdges()");

        std::vector<size_t> outgoing;
        for (const auto& edge : adjList[v]) {
            outgoing.push_back(edge.first);
        }
        return outgoing;
    }

    std::vector<size_t> getIncomingEdges(size_t v) const {
        std::shared_lock lock(graphMutex);
        if (v >= _vertices)
            throw std::out_of_range("Vertex index out of range in getIncomingEdges()");

        std::vector<size_t> incoming;
        for (size_t i = 0; i < _vertices; ++i) {
            if (hasEdge_internal(i, v))
                incoming.push_back(i);
        }
        return incoming;
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

        std::vector<EdgeList> reverseAdjList(_vertices);
        for (size_t u = 0; u < _vertices; ++u) {
            for (const auto& edge : adjList[u]) {
                reverseAdjList[edge.first].push_back(u, edge.second);
            }
        }

        visited.assign(_vertices, false);
        auto reverseDfs = [&reverseAdjList](size_t start, std::vector<bool>& visited) {
            std::stack<size_t> stack;
            stack.push(start);

            while (!stack.empty()) {
                size_t current = stack.top();
                stack.pop();

                if (!visited[current]) {
                    visited[current] = true;
                    for (const auto& edge : reverseAdjList[current]) {
                        stack.push(edge.first);
                    }
                }
            }
            };

        reverseDfs(0, visited);

        if (std::any_of(visited.begin(), visited.end(), [](bool v) { return !v; })) {
            return false; 
        }

        return true; 
    }

    bool detectCycle() const {
        std::shared_lock lock(graphMutex);
        std::vector<bool> visited(_vertices, false);
        std::vector<bool> recursionStack(_vertices, false);

        for (size_t i = 0; i < _vertices; ++i) {
            if (detectCycleUtil(i, visited, recursionStack))
                return true;
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

    // Non-const iterator for the DirectedGraph
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

    // Const iterator for the DirectedGraph
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

    bool detectCycleUtil(size_t v, std::vector<bool>& visited,
        std::vector<bool>& recursionStack) const {
        if (!visited[v]) {
            visited[v] = true;
            recursionStack[v] = true;

            for (const auto& edge : adjList[v]) {
                if (!visited[edge.first] && detectCycleUtil(edge.first, visited, recursionStack))
                    return true;
                else if (recursionStack[edge.first])
                    return true;
            }
        }
        recursionStack[v] = false;
        return false;
    }

    std::vector<EdgeList> adjList;
    std::atomic<size_t> _vertices;
    std::atomic<size_t> _edges;
    mutable std::shared_mutex graphMutex;
};