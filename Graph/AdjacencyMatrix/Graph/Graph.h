#pragma once
#include <vector>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <optional>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <algorithm>

template<typename T>
class Graph {
public:
    using Vertex = std::vector<std::optional<T>>;
    using Matrix = std::vector<Vertex>;

    // Iterator types for outer container (vertices) and inner container (neighbors)
    using iterator = typename Matrix::iterator;
    using const_iterator = typename Matrix::const_iterator;
    using inner_iterator = typename Vertex::iterator;
    using const_inner_iterator = typename Vertex::const_iterator;

    static constexpr size_t DEFAULT_VERTICES = 5;
    static constexpr size_t DEFAULT_EDGES = 0;
    static constexpr std::optional<T> DEFAULT_VALUE = std::nullopt;
    static constexpr size_t TERMINAL_VISIBLE_VERTICES = 17;
    static constexpr size_t ACCURACY = 0;

    // Constructors and destructor
    Graph() : Graph(DEFAULT_VERTICES) {}

    explicit Graph(const size_t& vertices) : _vertices(vertices), _edges(DEFAULT_EDGES) {
        Initialization();
    }

    explicit Graph(const Matrix& other) : _edges(DEFAULT_EDGES) {
        if (!Copy(other)) {
            setVertices(DEFAULT_VERTICES);
            Initialization();
        }
    }

    // Copy constructor
    Graph(const Graph& other) {
        std::shared_lock vLock(other.vertexMutex);
        std::shared_lock eLock(other.edgeMutex);
        _vertices = other._vertices.load();
        _edges = other._edges.load();
        _adjacency = other._adjacency;
    }

    // Move constructor
    Graph(Graph&& other) noexcept {
        std::unique_lock vLock(other.vertexMutex);
        std::unique_lock eLock(other.edgeMutex);
        _adjacency = std::move(other._adjacency);
        _vertices = other._vertices.load();
        _edges = other._edges.load();
        other._vertices = 0;
        other._edges = 0;
    }

    // Copy assignment operator
    Graph& operator=(const Graph& other) {
        if (this != &other) {
            std::unique_lock lock1(vertexMutex, std::defer_lock);
            std::unique_lock lock2(edgeMutex, std::defer_lock);
            std::shared_lock ovLock(other.vertexMutex, std::defer_lock);
            std::shared_lock oeLock(other.edgeMutex, std::defer_lock);
            std::lock(lock1, lock2, ovLock, oeLock);
            _vertices = other._vertices.load();
            _edges = other._edges.load();
            _adjacency = other._adjacency;
        }
        return *this;
    }

    // Move assignment operator
    Graph& operator=(Graph&& other) noexcept {
        if (this != &other) {
            std::unique_lock lock1(vertexMutex, std::defer_lock);
            std::unique_lock lock2(edgeMutex, std::defer_lock);
            std::unique_lock ovLock(other.vertexMutex, std::defer_lock);
            std::unique_lock oeLock(other.edgeMutex, std::defer_lock);
            std::lock(lock1, lock2, ovLock, oeLock);
            _adjacency = std::move(other._adjacency);
            _vertices = other._vertices.load();
            _edges = other._edges.load();
            other._vertices = 0;
            other._edges = 0;
        }
        return *this;
    }

    ~Graph() = default;

    // Member functions
    void addVertex() {
        std::unique_lock vLock(vertexMutex);
        std::unique_lock eLock(edgeMutex);
        for (auto& row : _adjacency) {
            row.push_back(DEFAULT_VALUE);
        }
        _vertices++;
        _adjacency.push_back(Vertex(_vertices.load(), DEFAULT_VALUE));
    }

    void removeVertex(size_t v) {
        std::unique_lock vLock(vertexMutex);
        std::unique_lock eLock(edgeMutex);
        if (!hasVertex_internal(v))
            throw std::out_of_range("Vertex index out of range in removeVertex()");
        _adjacency.erase(_adjacency.begin() + v);
        for (auto& row : _adjacency) {
            row.erase(row.begin() + v);
        }
        _vertices--;
    }

    bool hasVertex(size_t v) const {
        std::shared_lock vLock(vertexMutex);
        return hasVertex_internal(v);
    }

    size_t getVerticesCount() const {
        std::shared_lock vLock(vertexMutex);
        return _vertices.load();
    }

    void addEdge(size_t u, size_t v, T weight) {
        {
            std::shared_lock vLock(vertexMutex);
            if (!hasVertex_internal(u) || !hasVertex_internal(v))
                throw std::out_of_range("Vertex index out of range in addEdge()");
        }
        {
            std::unique_lock eLock(edgeMutex);
            if (hasEdge_internal(u, v))
                throw std::logic_error("Edge already exists in addEdge()");
            _adjacency[u][v] = weight;
            _edges++;
        }
    }

    void removeEdge(size_t u, size_t v) {
        {
            std::shared_lock vLock(vertexMutex);
            if (!hasVertex_internal(u) || !hasVertex_internal(v))
                throw std::out_of_range("Vertex index out of range in removeEdge()");
        }
        {
            std::unique_lock eLock(edgeMutex);
            if (!hasEdge_internal(u, v))
                throw std::logic_error("Edge does not exist in removeEdge()");
            _adjacency[u][v] = DEFAULT_VALUE;
            _edges--;
        }
    }

    void changeEdge(size_t u, size_t v, T weight) {
        {
            std::shared_lock vLock(vertexMutex);
            if (!hasVertex_internal(u) || !hasVertex_internal(v))
                throw std::out_of_range("Vertex index out of range in changeEdge()");
        }
        {
            std::unique_lock eLock(edgeMutex);
            if (!hasEdge_internal(u, v))
                throw std::logic_error("Edge does not exist in changeEdge()");
            _adjacency[u][v] = weight;
        }
    }

    bool hasEdge(size_t u, size_t v) const {
        std::shared_lock vLock(vertexMutex);
        if (!hasVertex_internal(u) || !hasVertex_internal(v))
            return false;
        std::shared_lock eLock(edgeMutex);
        return hasEdge_internal(u, v);
    }

    // Returns indices of vertices adjacent from vertex v.
    std::vector<size_t> getOutgoingEdges(size_t v) const {
        std::shared_lock vLock(vertexMutex);
        if (!hasVertex_internal(v))
            throw std::out_of_range("Vertex index out of range in getOutgoingEdges()");
        std::shared_lock eLock(edgeMutex);
        std::vector<size_t> outgoing;
        for (size_t j = 0; j < _vertices.load(); ++j) {
            if (hasEdge_internal(v, j))
                outgoing.push_back(j);
        }
        return outgoing;
    }

    // Returns count of outgoing edges from vertex v.
    size_t getOutgoingEdgesCount(size_t v) const {
        std::shared_lock vLock(vertexMutex);
        if (!hasVertex_internal(v))
            throw std::out_of_range("Vertex index out of range in getOutgoingEdgesCount()");
        std::shared_lock eLock(edgeMutex);
        return std::count_if(_adjacency[v].begin(), _adjacency[v].end(),
            [](const std::optional<T>& value) { return value.has_value(); });
    }

    // Returns indices of vertices with incoming edges to vertex v.
    std::vector<size_t> getIncomingEdges(size_t v) const {
        std::shared_lock vLock(vertexMutex);
        if (!hasVertex_internal(v))
            throw std::out_of_range("Vertex index out of range in getIncomingEdges()");
        std::shared_lock eLock(edgeMutex);
        std::vector<size_t> incoming;
        for (size_t i = 0; i < _vertices.load(); ++i) {
            if (hasEdge_internal(i, v))
                incoming.push_back(i);
        }
        return incoming;
    }

    // Returns count of incoming edges to vertex v.
    size_t getIncomingEdgesCount(size_t v) const {
        std::shared_lock vLock(vertexMutex);
        if (!hasVertex_internal(v))
            throw std::out_of_range("Vertex index out of range in getIncomingEdgesCount()");
        std::shared_lock eLock(edgeMutex);
        size_t count = 0;
        for (size_t i = 0; i < _vertices.load(); ++i) {
            if (hasEdge_internal(i, v))
                ++count;
        }
        return count;
    }

    // Returns the weight of edge from u to v, or std::nullopt if edge doesn't exist.
    std::optional<T> getWeight(size_t u, size_t v) const {
        std::shared_lock vLock(vertexMutex);
        if (!hasVertex_internal(u) || !hasVertex_internal(v))
            throw std::out_of_range("Vertex index out of range in getWeight()");
        std::shared_lock eLock(edgeMutex);
        if (!hasEdge_internal(u, v))
            return std::nullopt;
        return _adjacency[u][v];
    }

    size_t getEdgesCount() const {
        std::shared_lock eLock(edgeMutex);
        return _edges.load();
    }

    // Cycle detection using DFS
    bool detectCycle() const {
        std::shared_lock vLock(vertexMutex);
        std::shared_lock eLock(edgeMutex);
        if (_vertices.load() == 0 || _edges.load() == 0)
            return false;
        std::vector<bool> visited(_vertices.load(), false);
        std::vector<bool> recStack(_vertices.load(), false);
        for (size_t vertex = 0; vertex < _vertices.load(); ++vertex) {
            if (!visited[vertex]) {
                if (dfsCycle(vertex, visited, recStack))
                    return true;
            }
        }
        return false;
    }

    // Detect self-loop in any vertex
    bool detectLoop() const {
        std::shared_lock vLock(vertexMutex);
        std::shared_lock eLock(edgeMutex);
        if (_vertices.load() == 0 || _edges.load() == 0)
            return false;
        for (size_t v = 0; v < _vertices.load(); ++v) {
            if (hasEdge_internal(v, v))
                return true;
        }
        return false;
    }

    // Check if vertex v has a self-loop
    bool hasLoop(size_t v) const {
        std::shared_lock vLock(vertexMutex);
        if (!hasVertex_internal(v))
            throw std::out_of_range("Vertex index out of range in hasLoop()");
        std::shared_lock eLock(edgeMutex);
        return hasEdge_internal(v, v);
    }

    // Returns indices of vertices with self-loops
    std::vector<size_t> getLoops() const {
        std::shared_lock vLock(vertexMutex);
        std::shared_lock eLock(edgeMutex);
        std::vector<size_t> loops;
        if (!detectLoop())
            return loops;
        for (size_t v = 0; v < _vertices.load(); ++v) {
            if (hasEdge_internal(v, v))
                loops.push_back(v);
        }
        return loops;
    }

    // Detect contour by checking trace of successive powers of the matrix
    bool detectContour() const {
        std::shared_lock vLock(vertexMutex);
        std::shared_lock eLock(edgeMutex);
        if (_vertices.load() == 0 || _edges.load() == 0)
            return false;
        Matrix matrix = _adjacency;
        while (!isZeroMatrix(matrix)) {
            if (Trace(matrix) != DEFAULT_VALUE)
                return true;
            matrix = MatrixMultiplication(matrix);
        }
        return false;
    }

    // Check if the graph is empty
    bool isEmpty() const {
        std::shared_lock vLock(vertexMutex);
        return _vertices.load() == 0;
    }

    // Check if the graph is connected (has at least one edge)
    bool isConnected() const {
        std::shared_lock eLock(edgeMutex);
        return _edges.load() > 0;
    }

    // Print matrix representation to std::cout
    void PrintMatrix() const {
        std::shared_lock vLock(vertexMutex);
        std::shared_lock eLock(edgeMutex);
        if (_vertices.load() == 0)
            return;
        size_t space = maxDoubleLength();
        size_t padding = maxVertexLength();
        std::cout << std::setw(static_cast<int>(space - padding)) << " ";
        for (size_t column = 0; column < _vertices.load(); ++column) {
            std::cout << std::setw(static_cast<int>(space)) << std::left << column << " ";
        }
        std::cout << std::endl;
        for (size_t row = 0; row < _vertices.load(); ++row) {
            std::cout << std::setw(static_cast<int>(padding)) << std::right << row;
            for (size_t column = 0; column < _vertices.load(); ++column) {
                if (_adjacency[row][column].has_value())
                    std::cout << std::setw(static_cast<int>(space))
                    << std::fixed << std::setprecision(ACCURACY) << _adjacency[row][column].value() << " ";
                else
                    std::cout << std::setw(static_cast<int>(space)) << "0" << " ";
            }
            std::cout << std::endl;
        }
    }

    // Print graph in a more descriptive format
    void PrintGraph() const {
        std::shared_lock vLock(vertexMutex);
        std::shared_lock eLock(edgeMutex);
        std::cout << "\nGraph representation\n";
        for (size_t i = 0; i < _adjacency.size(); ++i) {
            std::cout << "V" << i;
            bool hasEdges = false;
            for (size_t j = 0; j < _adjacency[i].size(); ++j) {
                if (_adjacency[i][j].has_value()) {
                    if (!hasEdges) {
                        std::cout << " -> ";
                        hasEdges = true;
                    }
                    std::cout << "[V" << j << " | W" << _adjacency[i][j].value() << "] ";
                }
            }
            std::cout << "\n";
        }
        std::cout << "\n";
    }

    // Iterators for outer container (vertices) are provided below
    iterator begin() { std::shared_lock vLock(vertexMutex); return _adjacency.begin(); }
    iterator end() { std::shared_lock vLock(vertexMutex); return _adjacency.end(); }
    const_iterator begin() const { std::shared_lock vLock(vertexMutex); return _adjacency.begin(); }
    const_iterator end() const { std::shared_lock vLock(vertexMutex); return _adjacency.end(); }
    const_iterator cbegin() const { std::shared_lock vLock(vertexMutex); return _adjacency.cbegin(); }
    const_iterator cend() const { std::shared_lock vLock(vertexMutex); return _adjacency.cend(); }

private:
    Matrix _adjacency;
    std::atomic<size_t> _vertices;
    std::atomic<size_t> _edges;
    mutable std::shared_mutex vertexMutex;
    mutable std::shared_mutex edgeMutex;

    void Initialization() {
        _adjacency = Matrix(_vertices.load(), Vertex(_vertices.load(), DEFAULT_VALUE));
    }

    bool Copy(const Matrix& other) {
        if (other.empty())
            return false;
        size_t cols = other[0].size();
        if (cols != other.size())
            return false;
        for (const auto& row : other) {
            if (row.size() != cols)
                return false;
        }
        _adjacency = other;
        _vertices = other.size();
        return true;
    }

    void setVertices(const size_t& count) {
        _vertices = (count > 0) ? count : DEFAULT_VERTICES;
    }

    // Helper to determine maximum printed length of T values
    size_t maxDoubleLength() const {
        if (_vertices.load() == 0)
            return 0;
        size_t maxLength = 0;
        for (const auto& row : _adjacency) {
            for (const auto& value : row) {
                std::ostringstream oss;
                if (value.has_value())
                    oss << std::fixed << value.value();
                else
                    oss << "0";
                maxLength = std::max(maxLength, oss.str().length());
            }
        }
        return maxLength;
    }

    // Helper to determine printed length of vertex labels
    size_t maxVertexLength() const {
        if (_vertices.load() == 0)
            return 0;
        std::ostringstream oss;
        oss << _vertices.load() - 1;
        return oss.str().length();
    }

    // Depth-first search to detect cycle
    bool dfsCycle(size_t vertex, std::vector<bool>& visited, std::vector<bool>& recStack) const {
        visited[vertex] = true;
        recStack[vertex] = true;
        for (size_t adjVertex = 0; adjVertex < _vertices.load(); ++adjVertex) {
            if (hasEdge_internal(vertex, adjVertex)) {
                if (!visited[adjVertex]) {
                    if (dfsCycle(adjVertex, visited, recStack))
                        return true;
                }
                else if (recStack[adjVertex]) {
                    return true;
                }
            }
        }
        recStack[vertex] = false;
        return false;
    }

    // Multiply two matrices
    Matrix MatrixMultiplication(const Matrix& matrix) const {
        Matrix result(_vertices.load(), Vertex(_vertices.load(), DEFAULT_VALUE));
        for (size_t i = 0; i < _vertices.load(); i++) {
            for (size_t j = 0; j < _vertices.load(); j++) {
                for (size_t k = 0; k < _vertices.load(); k++) {
                    if (matrix[i][k].has_value() && matrix[k][j].has_value()) {
                        T prod = matrix[i][k].value() * matrix[k][j].value();
                        if (result[i][j].has_value())
                            result[i][j] = result[i][j].value() + prod;
                        else
                            result[i][j] = prod;
                    }
                }
            }
        }
        return result;
    }

    // Compute the trace of the matrix (sum of diagonal elements)
    T Trace(const Matrix& matrix) const {
        T trace = T();
        for (size_t i = 0; i < _vertices.load(); i++) {
            if (matrix[i][i].has_value())
                trace += matrix[i][i].value();
        }
        return trace;
    }

    // Check if the matrix is a zero matrix
    bool isZeroMatrix(const Matrix& matrix) const {
        for (const auto& row : matrix) {
            if (!std::all_of(row.begin(), row.end(), [](const std::optional<T>& value) { return !value.has_value() || value.value() == T(); }))
                return false;
        }
        return true;
    }

    bool hasVertex_internal(size_t v) const {
        return v < _vertices.load();
    }

    bool hasEdge_internal(size_t u, size_t v) const {
        return _adjacency[u][v].has_value();
    }
};
