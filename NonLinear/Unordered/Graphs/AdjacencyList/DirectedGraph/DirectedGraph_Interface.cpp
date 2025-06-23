#include "DirectedGraph.hpp"
#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
#include <exception>

using namespace std;

bool isNumber(const std::string& str) {
    if (str.empty()) return false;

    if (str[0] == '-') {
        if (str.length() == 1) return false;
        return std::all_of(str.begin() + 1, str.end(), ::isdigit);
    }

    return std::all_of(str.begin(), str.end(), ::isdigit);
}

bool isDouble(const std::string& str) {
    if (str.empty()) return false;
    size_t startIndex = 0;
    bool decimalPointFound = false;
    if (str[0] == '-') {
        if (str.length() == 1) return false;
        startIndex = 1;
    }
    for (size_t i = startIndex; i < str.length(); i++) {
        if (str[i] == '.') {
            if (decimalPointFound || i == startIndex || i == str.length() - 1) {
                return false;
            }
            decimalPointFound = true;
        }
        else if (!isdigit(str[i])) {
            return false;
        }
    }
    return true;
}

void instructions() {
    cout << "Enter one of the following commands:\n"
        << " 1 to add a vertex\n"
        << " 2 to remove a vertex\n"
        << " 3 to add an edge\n"
        << " 4 to remove an edge\n"
        << " 5 to change an edge\n"
        << " 6 to search for an edge\n"
        << " 7 to get the number of vertices\n"
        << " 8 to get the number of edges\n"
        << " 9 to get the outgoing edges\n"
        << "10 to get the incoming edges\n"
        << "11 to check for cycles\n"
        << "12 to print the graph\n"
        << "13 to check if the graph is empty\n"
        << "14 to check if the graph is connected\n"
        << "15 to iterate over all edges\n"
        << "16 to exit the program\n";
}

const size_t CHOICE = 16;

int main() {
    size_t numVertices;
    string input;
    cout << "Enter the initial number of vertices in the graph: ";
    cin >> input;
    while (!isNumber(input) || (numVertices = stoul(input)) < 1) {
        cout << "Number of vertices must be at least 1. Please enter a valid number: ";
        cin >> input;
    }

    DirectedGraph<double> graph(numVertices);

    int choice;
    size_t u, v;
    double weight;
    vector<size_t> edges;

    instructions();
    cout << "What would you like to do? ";

    while (true) {
        cin >> input;

        if (!input.empty() && all_of(input.begin(), input.end(), ::isdigit)) {
            choice = stoi(input);
            if (choice == CHOICE) break;
        }
        else {
            cout << "Invalid choice. Please enter a number between 1 and " << CHOICE << " : ";
            continue;
        }

        try {
            switch (choice) {
            case 1:
                graph.addVertex();
                cout << "Vertex added. Current number of vertices: " << graph.getVerticesCount() << endl;
                break;
            case 2:
                if (graph.isEmpty()) {
                    cout << "Graph is empty. No vertex to remove.\n";
                    break;
                }
                cout << "Enter the vertex to remove: ";
                cin >> input;
                while (!isNumber(input) || (u = stoul(input)) >= graph.getVerticesCount()) {
                    cout << "Invalid vertex. Please enter a valid vertex to remove: ";
                    cin >> input;
                }
                graph.removeVertex(u);
                cout << "Vertex " << u << " and its edges have been removed.\n";
                break;
            case 3:
                if (graph.isEmpty()) {
                    cout << "Graph is empty. Add vertices first.\n";
                    break;
                }
                cout << "Enter the starting vertex: ";
                cin >> input;
                while (!isNumber(input) || (u = stoul(input)) >= graph.getVerticesCount()) {
                    cout << "Invalid vertex. Please enter a valid starting vertex: ";
                    cin >> input;
                }
                cout << "Enter the ending vertex: ";
                cin >> input;
                while (!isNumber(input) || (v = stoul(input)) >= graph.getVerticesCount()) {
                    cout << "Invalid vertex. Please enter a valid ending vertex: ";
                    cin >> input;
                }
                cout << "Enter the weight of the edge: ";
                cin >> input;
                while (!isDouble(input)) {
                    cout << "Weight must be a number. Please enter a valid weight: ";
                    cin >> input;
                }
                weight = stod(input);
                graph.addEdge(u, v, weight);
                cout << "Edge added from vertex " << u << " to vertex " << v << " with weight " << weight << ".\n";
                break;
            case 4:
                if (graph.isEmpty() || !graph.hasEdges()) {
                    cout << "Graph has no edges to remove.\n";
                    break;
                }
                cout << "Enter the starting vertex: ";
                cin >> input;
                while (!isNumber(input) || (u = stoul(input)) >= graph.getVerticesCount()) {
                    cout << "Invalid vertex. Please enter a valid starting vertex: ";
                    cin >> input;
                }
                cout << "Enter the ending vertex: ";
                cin >> input;
                while (!isNumber(input) || (v = stoul(input)) >= graph.getVerticesCount()) {
                    cout << "Invalid vertex. Please enter a valid ending vertex: ";
                    cin >> input;
                }
                graph.removeEdge(u, v);
                cout << "Edge removed from vertex " << u << " to vertex " << v << ".\n";
                break;
            case 5:
                if (graph.isEmpty() || !graph.hasEdges()) {
                    cout << "Graph has no edges to change.\n";
                    break;
                }
                cout << "Enter the starting vertex: ";
                cin >> input;
                while (!isNumber(input) || (u = stoul(input)) >= graph.getVerticesCount()) {
                    cout << "Invalid vertex. Please enter a valid starting vertex: ";
                    cin >> input;
                }
                cout << "Enter the ending vertex: ";
                cin >> input;
                while (!isNumber(input) || (v = stoul(input)) >= graph.getVerticesCount()) {
                    cout << "Invalid vertex. Please enter a valid ending vertex: ";
                    cin >> input;
                }
                cout << "Enter the new weight of the edge: ";
                cin >> input;
                while (!isDouble(input)) {
                    cout << "Weight must be a number. Please enter a valid weight: ";
                    cin >> input;
                }
                weight = stod(input);
                graph.changeEdge(u, v, weight);
                cout << "Edge updated from vertex " << u << " to vertex " << v << " with new weight " << weight << ".\n";
                break;
            case 6:
                cout << "Enter the starting vertex: ";
                cin >> input;
                while (!isNumber(input) || (u = stoul(input)) >= graph.getVerticesCount()) {
                    cout << "Invalid vertex. Please enter a valid starting vertex: ";
                    cin >> input;
                }
                cout << "Enter the ending vertex: ";
                cin >> input;
                while (!isNumber(input) || (v = stoul(input)) >= graph.getVerticesCount()) {
                    cout << "Invalid vertex. Please enter a valid ending vertex: ";
                    cin >> input;
                }
                if (graph.hasEdge(u, v)) {
                    cout << "Edge exists from vertex " << u << " to vertex " << v << " with weight " << graph.getEdgeWeight(u, v) << ".\n";
                }
                else {
                    cout << "Edge not found.\n";
                }
                break;
            case 7:
                cout << "Number of vertices in the graph: " << graph.getVerticesCount() << ".\n";
                break;
            case 8:
                cout << "Number of edges in the graph: " << graph.getEdgesCount() << ".\n";
                break;
            case 9:
                cout << "Enter the vertex to get the outgoing edges: ";
                cin >> input;
                while (!isNumber(input) || (u = stoul(input)) >= graph.getVerticesCount()) {
                    cout << "Invalid vertex. Please enter a valid vertex: ";
                    cin >> input;
                }
                edges = graph.getOutgoingEdges(u);
                if (edges.empty()) {
                    cout << "Vertex " << u << " has no outgoing edges.\n";
                }
                else {
                    cout << "Outgoing edges from vertex " << u << ": ";
                    for (size_t vertex : edges) {
                        cout << vertex << " ";
                    }
                    cout << "\n";
                }
                break;
            case 10:
                cout << "Enter the vertex to get the incoming edges: ";
                cin >> input;
                while (!isNumber(input) || (u = stoul(input)) >= graph.getVerticesCount()) {
                    cout << "Invalid vertex. Please enter a valid vertex: ";
                    cin >> input;
                }
                edges = graph.getIncomingEdges(u);
                if (edges.empty()) {
                    cout << "Vertex " << u << " has no incoming edges.\n";
                }
                else {
                    cout << "Incoming edges to vertex " << u << ": ";
                    for (size_t vertex : edges) {
                        cout << vertex << " ";
                    }
                    cout << "\n";
                }
                break;
            case 11:
                cout << (graph.detectCycle() ? "The graph contains a cycle.\n" : "No cycle detected in the graph.\n");
                break;
            case 12:
                graph.printGraph();
                break;
            case 13:
                cout << (graph.isEmpty() ? "The graph is empty.\n" : "The graph is not empty.\n");
                break;
            case 14:
                cout << (graph.isConnected() ? "The graph is connected.\n" : "The graph is not connected.\n");
                break;
            case 15:
                cout << "Iterating over all edges in the graph:\n";
                for (const auto& edge : graph) {
                    cout << "Edge from " << edge.first << " with weight " << edge.second << "\n";
                }
                break;
            default:
                cout << "Invalid choice. Please try again.\n";
                break;
            }
        }
        catch (const std::exception& e) {
            cout << "Error: " << e.what() << "\n";
        }

        cout << "What would you like to do next? ";
    }

    cout << "End of the program.\n";
    return 0;
}
