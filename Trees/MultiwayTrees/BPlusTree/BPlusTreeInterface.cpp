#include "BPlusTree.h"
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
        << " 7 to perform a range query\n"
        << " 8 to print the tree structure\n"
        << " 9 to check if the tree is empty\n"
        << "10 to exit the program\n";
}

const size_t CHOICE = 10;

int main() {
    BPlusTree<int, string> tree;
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
            cout << "Enter key to insert: ";
            cin >> input;
            while (!isNumber(input)) {
                cout << "Key must be a number. Please enter a valid key: ";
                cin >> input;
            }
            int key = std::stoi(input);

            cout << "Enter value to insert: ";
            string value;
            cin >> value;

            if (tree.insert(key, value)) {
                cout << "Element (" << key << ", " << value << ") successfully inserted.\n";
            }
            else {
                cout << "Key " << key << " already exists.\n";
            }
            break;
        }
        case 2: {
            if (tree.empty()) {
                cout << "The tree is empty, cannot delete an element.\n";
                break;
            }
            cout << "Enter key to delete: ";
            cin >> input;
            while (!isNumber(input)) {
                cout << "Key must be a number. Please enter a valid key: ";
                cin >> input;
            }
            int key = std::stoi(input);
            if (tree.remove(key)) {
                cout << "Element with key " << key << " successfully deleted.\n";
            }
            else {
                cout << "Key " << key << " not found.\n";
            }
            break;
        }
        case 3: {
            if (tree.empty()) {
                cout << "The tree is empty, cannot search for an element.\n";
                break;
            }
            cout << "Enter key to search: ";
            cin >> input;
            while (!isNumber(input)) {
                cout << "Key must be a number. Please enter a valid key: ";
                cin >> input;
            }
            int key = std::stoi(input);
            auto value = tree.find(key);
            if (value.has_value()) {
                cout << "Element found: (" << key << ", " << value.value() << ").\n";
            }
            else {
                cout << "Key " << key << " not found.\n";
            }
            break;
        }
        case 4: {
            cout << "Number of elements in the tree: " << tree.size() << "\n";
            break;
        }
        case 5: {
            tree.clear();
            cout << "The tree has been cleared.\n";
            break;
        }
        case 6: {
            if (tree.empty()) {
                cout << "The tree is empty, cannot display elements.\n";
            }
            else {
                cout << "Elements in Inorder Traversal:\n";
                tree.traverse([](const int& key, const string& value) {
                    cout << "(" << key << ", " << value << ") ";
                    });
                cout << "\n";
            }
            break;
        }
        case 7: {
            if (tree.empty()) {
                cout << "The tree is empty, cannot perform a range query.\n";
                break;
            }

            string inputLow, inputHigh;
            int low, high;

            cout << "Enter low value for range query: ";
            cin >> inputLow;
            while (!isNumber(inputLow)) {
                cout << "Value must be a number. Please enter a valid low value: ";
                cin >> inputLow;
            }
            low = std::stoi(inputLow);

            cout << "Enter high value for range query: ";
            cin >> inputHigh;
            while (!isNumber(inputHigh)) {
                cout << "Value must be a number. Please enter a valid high value: ";
                cin >> inputHigh;
            }
            high = std::stoi(inputHigh);

            if (low > high) {
                std::swap(low, high);
            }

            auto result = tree.rangeQuery(low, high);
            if (result.empty()) {
                cout << "No elements found in this range.\n";
            }
            else {
                cout << "Elements in the range [" << low << ", " << high << "]: ";
                for (const auto& [key, value] : result) {
                    cout << "(" << key << ", " << value << ") ";
                }
                cout << "\n";
            }
            break;
        }
        case 8: {
            if (tree.empty()) {
                cout << "The tree is empty, cannot display structure.\n";
            }
            else {
                tree.printTree();
            }
            break;
        }
        case 9: {
            if (tree.empty()) {
                cout << "The tree is empty.\n";
            }
            else {
                cout << "The tree is not empty.\n";
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