#include "DoublyLinkedList.hpp"
#include <iostream>
#include <string>
#include <utility>
#include <algorithm>

using namespace std;

bool isNumber(const std::string& str) {
    if (str.empty()) return false;

    if (str[0] == '-') {
        if (str.length() == 1) return false; 
        return std::all_of(str.begin() + 1, str.end(), ::isdigit);
    }

    return std::all_of(str.begin(), str.end(), ::isdigit);
}

void instructions() {
    cout << "Enter one of the following commands:\n"
        << " 1 to insert at the front\n"
        << " 2 to insert at the back\n"
        << " 3 to remove from the front\n"
        << " 4 to remove from the back\n"
        << " 5 to display the list\n"
        << " 6 to search for a key\n"
        << " 7 to get the size of the list\n"
        << " 8 to check if the list is empty\n"
        << " 9 to insert at a specific position\n"
        << "10 to erase an element by position\n"
        << "11 to erase a range of elements\n"
        << "12 to access an element by index\n"
        << "13 to check for cycles in the list\n"
        << "14 to exit the program\n";
}

const size_t CHOICE = 14;

int main() {
    DoublyLinkedList<int, string> list;
    int choice;
    string input;
    int key, pos, first, last, index;
    string value;

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

        switch (choice) {
        case 1: {
            cout << "Enter key and value to insert at the front (e.g., 1 hello): ";
            cin >> input;
            while (!isNumber(input)) {
                cout << "Key must be a number. Please enter a valid key: ";
                cin >> input;
            }
            key = std::stoi(input);
            cin >> value;
            list.push_front(key, value);
            cout << "Inserted (" << key << ", " << value << ") at the front.\n";
            break;
        }
        case 2: {
            cout << "Enter key and value to insert at the back (e.g., 1 hello): ";
            cin >> input;
            while (!isNumber(input)) {
                cout << "Key must be a number. Please enter a valid key: ";
                cin >> input;
            }
            key = std::stoi(input);
            cin >> value;
            list.push_back(key, value);
            cout << "Inserted (" << key << ", " << value << ") at the back.\n";
            break;
        }
        case 3: {
            auto result = list.pop_front();
            if (result) {
                cout << "Removed (" << result->first << ", " << result->second << ") from the front.\n";
            }
            else {
                cout << "The list is empty.\n";
            }
            break;
        }
        case 4: {
            auto result = list.pop_back();
            if (result) {
                cout << "Removed (" << result->first << ", " << result->second << ") from the back.\n";
            }
            else {
                cout << "The list is empty.\n";
            }
            break;
        }
        case 5: {
            cout << "List contents: ";
            list.print();
            break;
        }
        case 6: {
            cout << "Enter key to search: ";
            cin >> input;
            while (!isNumber(input)) {
                cout << "Key must be a number. Please enter a valid key: ";
                cin >> input;
            }
            key = std::stoi(input);
            auto result = list.search(key);
            if (result) {
                cout << "Found (" << result->get().first << ", " << result->get().second << ") in the list.\n";
            }
            else {
                cout << "Key " << key << " not found in the list.\n";
            }
            break;
        }
        case 7: {
            cout << "Size of the list: " << list.size() << "\n";
            break;
        }
        case 8: {
            if (list.empty()) {
                cout << "The list is empty.\n";
            }
            else {
                cout << "The list is not empty.\n";
            }
            break;
        }
        case 9: {
            cout << "Enter position, key, and value to insert (e.g., 2 1 hello): ";
            cin >> input;
            while (!isNumber(input)) {
                cout << "Position must be a number. Please enter a valid position: ";
                cin >> input;
            }
            pos = std::stoi(input);
            cin >> input;
            while (!isNumber(input)) {
                cout << "Key must be a number. Please enter a valid key: ";
                cin >> input;
            }
            key = std::stoi(input);
            cin >> value;
            try {
                list.insert(pos, key, value);
                cout << "Inserted (" << key << ", " << value << ") at position " << pos << ".\n";
            }
            catch (const out_of_range& e) {
                cout << e.what() << "\n";
            }
            break;
        }
        case 10: {
            cout << "Enter position to erase: ";
            cin >> input;
            while (!isNumber(input)) {
                cout << "Position must be a number. Please enter a valid position: ";
                cin >> input;
            }
            pos = std::stoi(input);
            try {
                list.erase(pos);
                cout << "Erased element at position " << pos << ".\n";
            }
            catch (const out_of_range& e) {
                cout << e.what() << "\n";
            }
            break;
        }
        case 11: {
            cout << "Enter range to erase (first last): ";
            cin >> input;
            while (!isNumber(input)) {
                cout << "First position must be a number. Please enter a valid position: ";
                cin >> input;
            }
            first = std::stoi(input);
            cin >> input;
            while (!isNumber(input)) {
                cout << "Last position must be a number. Please enter a valid position: ";
                cin >> input;
            }
            last = std::stoi(input);
            try {
                list.erase(first, last);
                cout << "Erased elements from position " << first << " to " << last << ".\n";
            }
            catch (const out_of_range& e) {
                cout << e.what() << "\n";
            }
            break;
        }
        case 12: {
            cout << "Enter index to access: ";
            cin >> input;
            while (!isNumber(input)) {
                cout << "Index must be a number. Please enter a valid index: ";
                cin >> input;
            }
            index = std::stoi(input);
            try {
                auto& element = list.at(index);
                cout << "Element at index " << index << ": (" << element.first << ", " << element.second << ")\n";
            }
            catch (const out_of_range& e) {
                cout << e.what() << "\n";
            }
            break;
        }
        case 13: {
            if (list.cycle()) {
                cout << "The list contains a cycle.\n";
            }
            else {
                cout << "The list does not contain a cycle.\n";
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
