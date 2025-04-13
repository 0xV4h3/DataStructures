#include "Graph.h"
#include <iostream>
#include <string>
#include <cctype>
#include <vector>

using namespace std;

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

bool isWholeNumber(const std::string& str) {
    if (str.empty()) return false;
    for (char c : str) {
        if (!isdigit(c))
            return false;
    }
    return true;
}

void instructions() {
    cout << "Enter one of the following options:\n"
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
        << "12 to check for contour\n"
        << "13 to check for loop\n"
        << "14 to get the list of vertices with a loop\n"
        << "15 to print the graph\n"
        << "16 to print the adjacency matrix\n"
        << "17 to end\n";
}

const size_t CHOICE = 17;

int main() {
    size_t numVertices;
    string input;
    cout << "Enter the initial number of vertices in the graph: ";
    cin >> input;
    while (!isWholeNumber(input) || (numVertices = stoul(input)) < 1) {
        cout << "Number of vertices must be at least 1. Please enter a valid number: ";
        cin >> input;
    }

    Graph<double> graph(numVertices);

    int choice;
    size_t u, v;
    double weight;
    vector<size_t> loops;
    vector<size_t> outgoingVertices;
    vector<size_t> incomingVertices;

    instructions();
    cout << "What do you want to do? ";

    while (true) {
        cin >> input;
        if (isWholeNumber(input)) {
            choice = stoi(input);
            if (choice == CHOICE) break;
        }
        else {
            cout << "Invalid choice. Please enter a number between 1 and " << CHOICE << " : ";
            continue;
        }

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
            while (!isWholeNumber(input) || (u = stoul(input)) >= graph.getVerticesCount()) {
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
            while (!isWholeNumber(input) || (u = stoul(input)) >= graph.getVerticesCount()) {
                cout << "Invalid vertex. Please enter a valid starting vertex: ";
                cin >> input;
            }
            cout << "Enter the ending vertex: ";
            cin >> input;
            while (!isWholeNumber(input) || (v = stoul(input)) >= graph.getVerticesCount()) {
                cout << "Invalid vertex. Please enter a valid ending vertex: ";
                cin >> input;
            }
            if (graph.hasEdge(u, v)) {
                cout << "An edge already exists between vertex " << u << " and vertex " << v << ".\n";
                cout << "Do you want to update the weight? (y/n): ";
                char response;
                cin >> response;
                if (response == 'y' || response == 'Y') {
                    cout << "Enter the new weight of the edge: ";
                    cin >> input;
                    while (!isDouble(input)) {
                        cout << "Weight must be a number. Please enter a valid weight: ";
                        cin >> input;
                    }
                    weight = stod(input);
                    graph.changeEdge(u, v, weight);
                    cout << "Edge updated between vertex " << u << " and vertex " << v << " with new weight " << weight << ".\n";
                }
                else {
                    cout << "Edge not modified.\n";
                }
            }
            else {
                cout << "Enter the weight of the edge: ";
                cin >> input;
                while (!isDouble(input)) {
                    cout << "Weight must be a number. Please enter a valid weight: ";
                    cin >> input;
                }
                weight = stod(input);
                graph.addEdge(u, v, weight);
                cout << "Edge added between vertex " << u << " and vertex " << v << " with weight " << weight << ".\n";
            }
            break;
        case 4:
            if (!graph.isConnected()) {
                cout << "Graph is not connected. No edge to remove.\n";
                break;
            }
            cout << "Enter the starting vertex: ";
            cin >> input;
            while (!isWholeNumber(input) || (u = stoul(input)) >= graph.getVerticesCount()) {
                cout << "Invalid vertex. Please enter a valid starting vertex: ";
                cin >> input;
            }
            cout << "Enter the ending vertex: ";
            cin >> input;
            while (!isWholeNumber(input) || (v = stoul(input)) >= graph.getVerticesCount()) {
                cout << "Invalid vertex. Please enter a valid ending vertex: ";
                cin >> input;
            }
            graph.removeEdge(u, v);
            cout << "Edge removed between vertex " << u << " and vertex " << v << ".\n";
            break;
        case 5:
            if (!graph.isConnected()) {
                cout << "Graph is not connected. No edge to change.\n";
                break;
            }
            cout << "Enter the starting vertex: ";
            cin >> input;
            while (!isWholeNumber(input) || (u = stoul(input)) >= graph.getVerticesCount()) {
                cout << "Invalid vertex. Please enter a valid starting vertex: ";
                cin >> input;
            }
            cout << "Enter the ending vertex: ";
            cin >> input;
            while (!isWholeNumber(input) || (v = stoul(input)) >= graph.getVerticesCount()) {
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
            cout << "Edge updated between vertex " << u << " and vertex " << v << " with new weight " << weight << ".\n";
            break;
        case 6:
            if (!graph.isConnected()) {
                cout << "Graph is not connected. No edges to search.\n";
                break;
            }
            cout << "Enter the starting vertex: ";
            cin >> input;
            while (!isWholeNumber(input) || (u = stoul(input)) >= graph.getVerticesCount()) {
                cout << "Invalid vertex. Please enter a valid starting vertex: ";
                cin >> input;
            }
            cout << "Enter the ending vertex: ";
            cin >> input;
            while (!isWholeNumber(input) || (v = stoul(input)) >= graph.getVerticesCount()) {
                cout << "Invalid vertex. Please enter a valid ending vertex: ";
                cin >> input;
            }
            if (graph.hasEdge(u, v)) {
                cout << "Edge exists between vertex " << u << " and vertex " << v << " with weight " << *graph.getWeight(u, v) << ".\n";
            }
            else {
                cout << "Edge not found.\n";
            }
            break;
        case 7:
            if (graph.isEmpty()) {
                cout << "Graph is empty.\n";
                break;
            }
            cout << "Number of vertices in the graph: " << graph.getVerticesCount() << ".\n";
            break;
        case 8:
            if (!graph.isConnected()) {
                cout << "Graph is not connected.\n";
                break;
            }
            cout << "Number of edges in the graph: " << graph.getEdgesCount() << ".\n";
            break;
        case 9:
            if (!graph.isConnected()) {
                cout << "Graph is not connected.\n";
                break;
            }
            cout << "Enter the vertex to get the outgoing edges: ";
            cin >> input;
            while (!isWholeNumber(input) || (u = stoul(input)) >= graph.getVerticesCount()) {
                cout << "Invalid vertex. Please enter a valid vertex: ";
                cin >> input;
            }
            outgoingVertices = graph.getOutgoingEdges(u);
            if (graph.getOutgoingEdgesCount(u) == 0) {
                cout << "Vertex " << u << " has no outgoing edges.\n";
                break;
            }
            cout << "Vertex " << u << " has " << graph.getOutgoingEdgesCount(u) << " outgoing edges to vertices: ";
            for (size_t vertex : outgoingVertices) {
                cout << vertex << " ";
            }
            cout << "\n";
            break;
        case 10:
            if (!graph.isConnected()) {
                cout << "Graph is not connected.\n";
                break;
            }
            cout << "Enter the vertex to get the incoming edges: ";
            cin >> input;
            while (!isWholeNumber(input) || (u = stoul(input)) >= graph.getVerticesCount()) {
                cout << "Invalid vertex. Please enter a valid vertex: ";
                cin >> input;
            }
            incomingVertices = graph.getIncomingEdges(u);
            if (graph.getIncomingEdgesCount(u) == 0) {
                cout << "Vertex " << u << " has no incoming edges.\n";
                break;
            }
            cout << "Vertex " << u << " has " << graph.getIncomingEdgesCount(u) << " incoming edges from vertices: ";
            for (size_t vertex : incomingVertices) {
                cout << vertex << " ";
            }
            cout << "\n";
            break;
        case 11:
            cout << (graph.detectCycle() ? "The graph contains a cycle.\n" : "No cycle detected in the graph.\n");
            break;
        case 12:
            cout << (graph.detectContour() ? "The graph contains a contour.\n" : "No contour detected in the graph.\n");
            break;
        case 13:
            if (!graph.detectLoop()) {
                cout << "No loop detected in the graph.\n";
                break;
            }
            cout << "The graph contains a loop.\n";
            break;
        case 14:
            if (!graph.detectLoop()) {
                cout << "No loop detected in the graph.\n";
                break;
            }
            loops = graph.getLoops();
            if (loops.size() == 1) {
                cout << "This vertex has a loop: " << loops[0] << "\n";
            }
            else {
                cout << "List of vertices with a loop: ";
                for (auto& vertex : loops) {
                    cout << vertex << "; ";
                }
                cout << "\n";
            }
            loops.clear();
            break;
        case 15:
            if (graph.isEmpty()) {
                cout << "Graph is empty.\n";
                break;
            }
            graph.PrintGraph();
            break;
        case 16:
            if (graph.isEmpty()) {
                cout << "Graph adjacency matrix is empty.\n";
                break;
            }
            graph.PrintMatrix();
            break;
        default:
            cout << "Invalid choice. Try again.\n";
            break;
        }
        cout << "What do you want to do next? ";
    }
    cout << "End of program.\n";
    return 0;
}
