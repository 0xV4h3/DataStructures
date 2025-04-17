#include "AVL.hpp"
#include <iostream>
#include <string>

using namespace std;

bool isPositiveNumber(const std::string& str) {
    for (char c : str) {
        if (!isdigit(c)) {
            return false;
        }
    }
    return true;
}

bool isNumber(const std::string& str) {
    if (str.empty()) return false;
    size_t startIndex = 0;

    if (str[0] == '-') {
        if (str.length() == 1) return false;
        startIndex = 1;
    }

    for (size_t i = startIndex; i < str.length(); i++) {
        if (!isdigit(str[i])) {
            return false;
        }
    }
    return true;
}


void instructions() {
    cout << "Enter one of the following commands:\n"
        << " 1 to insert an element\n"
        << " 2 to delete an element\n"
        << " 3 to search for an element\n"
        << " 4 to get the number of elements\n"
        << " 5 to clear the tree\n"
        << " 6 to display elements in Inorder Traversal\n"
        << " 7 to display elements in Preorder Traversal\n"
        << " 8 to display elements in Postorder Traversal\n"
        << " 9 to display elements in Level Order Traversal\n"
        << "10 to display elements in Reverse Level Order Traversal\n"
        << "11 to display elements in Boundary Traversal\n"
        << "12 to display elements in Diagonal Traversal\n"
        << "13 to perform Range Search\n"
        << "14 to get the depth of the tree\n"
        << "15 to check if the tree is empty\n"
        << "16 to check if the tree is full\n"
        << "17 to check if the tree is complete\n"
        << "18 to check if the tree is perfect\n"
        << "19 to check if the tree is balanced\n"
        << "20 to check if the tree is degenerate\n"
        << "21 to visualize the tree\n"
        << "22 to display the minimum key\n"
        << "23 to display the maximum key\n"
        << "24 to exit the program\n";
}

const size_t CHOICE = 24;

int main() {
    AVL<int> tree;
    string input;
    int choice;

    instructions();
    cout << "What would you like to do? ";

    while (true) {
        cin >> input;

        if (isPositiveNumber(input)) {
            choice = std::stoi(input);
            if (choice == CHOICE) break;
        }
        else {
            cout << "Invalid choice. Please enter a number between 1 and " << CHOICE << " : ";
            continue;
        }

        switch (choice) {
        case 1: {
            cout << "Enter value to insert: ";
            cin >> input;
            while (!isNumber(input)) {
                cout << "Value must be a number. Please enter a valid value: ";
                cin >> input;
            }
            int key = std::stoi(input);
            if (tree.Insert(key)) {
                cout << "Element " << key << " successfully inserted.\n";
            }
            else {
                cout << "Element " << key << " already exists.\n";
            }
            break;
        }
        case 2: {
            if (tree.isEmpty()) {
                cout << "The tree is empty, cannot delete an element.\n";
                break;
            }
            cout << "Enter value to delete: ";
            cin >> input;
            while (!isNumber(input)) {
                cout << "Value must be a number. Please enter a valid value: ";
                cin >> input;
            }
            int key = std::stoi(input);
            if (tree.Delete(key)) {
                cout << "Element " << key << " successfully deleted.\n";
            }
            else {
                cout << "Element " << key << " not found.\n";
            }
            break;
        }
        case 3: {
            if (tree.isEmpty()) {
                cout << "The tree is empty, cannot search for an element.\n";
                break;
            }
            cout << "Enter value to search: ";
            cin >> input;
            while (!isNumber(input)) {
                cout << "Value must be a number. Please enter a valid value: ";
                cin >> input;
            }
            int key = std::stoi(input);
            auto foundNode = tree.Search(key).get();
            if (foundNode) {
                cout << "Element " << key << " found.\n";
            }
            else {
                cout << "Element " << key << " not found.\n";
            }
            break;
        }
        case 4: {
            if (tree.isEmpty()) {
                cout << "The tree is empty, cannot display the number of elements.\n";
            }
            else {
                cout << "Number of elements in the tree: " << tree.getNodeCount() << "\n";
            }
            break;
        }
        case 5: {
            tree.~AVL();
            cout << "The tree has been cleared.\n";
            break;
        }
        case 6: {
            if (tree.isEmpty()) {
                cout << "The tree is empty, cannot display elements.\n";
            }
            else {
                cout << "Elements in Inorder Traversal: " << tree.toStringInorder() << "\n";
            }
            break;
        }
        case 7: {
            if (tree.isEmpty()) {
                cout << "The tree is empty, cannot display elements.\n";
            }
            else {
                cout << "Elements in Preorder Traversal: " << tree.toStringPreorder() << "\n";
            }
            break;
        }
        case 8: {
            if (tree.isEmpty()) {
                cout << "The tree is empty, cannot display elements.\n";
            }
            else {
                cout << "Elements in Postorder Traversal: " << tree.toStringPostorder() << "\n";
            }
            break;
        }
        case 9: {
            if (tree.isEmpty()) {
                cout << "The tree is empty, cannot display elements.\n";
            }
            else {
                cout << "Elements in Level Order Traversal: " << tree.toStringLevelOrder() << "\n";
            }
            break;
        }
        case 10: {
            if (tree.isEmpty()) {
                cout << "The tree is empty, cannot display elements.\n";
            }
            else {
                cout << "Elements in Reverse Level Order Traversal: " << tree.toStringReverseLevelOrder() << "\n";
            }
            break;
        }
        case 11: {
            if (tree.isEmpty()) {
                cout << "The tree is empty, cannot display elements.\n";
            }
            else {
                cout << "Elements in Boundary Traversal: " << tree.toStringBoundary() << "\n";
            }
            break;
        }
        case 12: {
            if (tree.isEmpty()) {
                cout << "The tree is empty, cannot display elements.\n";
            }
            else {
                cout << "Elements in Diagonal Traversal: " << tree.toStringDiagonal() << "\n";
            }
            break;
        }
        case 13: {
            if (tree.isEmpty()) {
                cout << "The tree is empty, cannot perform range search.\n";
                break;
            }

            string inputLow, inputHigh;
            int low, high;

            cout << "Enter low value for range search: ";
            cin >> inputLow;
            while (!isNumber(inputLow)) {
                cout << "Value must be a number. Please enter a valid low value: ";
                cin >> inputLow;
            }
            low = std::stoi(inputLow);

            cout << "Enter high value for range search: ";
            cin >> inputHigh;
            while (!isNumber(inputHigh)) {
                cout << "Value must be a number. Please enter a valid high value: ";
                cin >> inputHigh;
            }
            high = std::stoi(inputHigh);

            if (low > high) {
                std::swap(low, high);
            }

            string result = tree.toStringRange(low, high);
            if (result.empty()) {
                cout << "No elements found in this range.\n";
            }
            else {
                cout << "Elements in the range [" << low << ", " << high << "]: ";
                cout << result << "\n";
            }
            break;
        }
        case 14: {
            if (tree.isEmpty()) {
                cout << "The tree is empty, cannot get depth.\n";
            }
            else {
                cout << "Tree depth: " << tree.getDepth() << "\n";
            }
            break;
        }
        case 15: {
            if (tree.isEmpty()) {
                cout << "The tree is empty.\n";
            }
            else {
                cout << "The tree is not empty.\n";
            }
            break;
        }
        case 16: {
            if (tree.isEmpty()) {
                cout << "The tree is empty, cannot check it for fullness.\n";
            }
            else {
                if (tree.isFull())
                    cout << "The tree is full.\n";
                else
                    cout << "The tree is not full.\n";
            }
            break;
        }
        case 17: {
            if (tree.isEmpty()) {
                cout << "The tree is empty, cannot check it for completeness.\n";
            }
            else {
                if (tree.isComplete())
                    cout << "The tree is complete.\n";
                else
                    cout << "The tree is not complete.\n";
            }
            break;
        }
        case 18: {
            if (tree.isEmpty()) {
                cout << "The tree is empty, cannot check it for perfection.\n";
            }
            else {
                if (tree.isPerfect())
                    cout << "The tree is perfect.\n";
                else
                    cout << "The tree is not perfect.\n";
            }
            break;
        }
        case 19: {
            if (tree.isEmpty()) {
                cout << "The tree is empty, cannot check it for balance.\n";
            }
            else {
                if (tree.isBalanced())
                    cout << "The tree is balanced.\n";
                else
                    cout << "The tree is not balanced.\n";
            }
            break;
        }
        case 20: {
            if (tree.isEmpty()) {
                cout << "The tree is empty, cannot check it for degeneracy.\n";
            }
            else {
                if (tree.isDegenerate())
                    cout << "The tree is degenerate.\n";
                else
                    cout << "The tree is not degenerate.\n";
            }
            break;
        }
        case 21: {
            if (tree.isEmpty()) {
                cout << "The tree is empty, cannot visualize.\n";
            }
            else {
                cout << "Tree visualization:\n";
                tree.visualize();
            }
            break;
        }
        case 22: {
            auto minKey = tree.MinimumKey();
            if (minKey) {
                cout << "Minimum key: " << *minKey << "\n";
            }
            else {
                cout << "The tree is empty, cannot display minimum key.\n";
            }
            break;
        }
        case 23: {
            auto maxKey = tree.MaximumKey();
            if (maxKey) {
                cout << "Maximum key: " << *maxKey << "\n";
            }
            else {
                cout << "The tree is empty, cannot display maximum key.\n";
            }
            break;
        }
        default:
            cout << "Invalid choice. Please try again.\n";
            break;
        }

        cout << "What would you like to do next? ";
    }

    cout << "End of the program.\n";
    return 0;
}