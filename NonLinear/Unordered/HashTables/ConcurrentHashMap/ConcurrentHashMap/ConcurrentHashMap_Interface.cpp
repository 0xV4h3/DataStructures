#include "ConcurrentHashMap.hpp"
#include <iostream>
#include <string>
#include <optional>
#include <algorithm>

using namespace std;

void instructions() {
    cout << "Enter one of the following commands:\n"
        << " 1 to insert a key-value pair\n"
        << " 2 to search for a key\n"
        << " 3 to remove a key\n"
        << " 4 to display all key-value pairs\n"
        << " 5 to get the number of elements\n"
        << " 6 to check if the map is empty\n"
        << " 7 to exit the program\n";
}

const size_t CHOICE = 7;

int main() {
    ConcurrentHashMap<string, string> map;
    string key, value, input;
    int choice;

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
            cout << "Enter key to insert: ";
            cin >> key;
            cout << "Enter value to insert: ";
            cin >> value;
            map.insert(key, value);
            cout << "Key \"" << key << "\" with value \"" << value << "\" successfully inserted.\n";
            break;
        }
        case 2: {
            cout << "Enter key to search: ";
            cin >> key;
            auto result = map.search(key);
            if (result.has_value()) {
                cout << "Key \"" << key << "\" found with value \"" << result.value() << "\".\n";
            }
            else {
                cout << "Key \"" << key << "\" not found.\n";
            }
            break;
        }
        case 3: {
            cout << "Enter key to remove: ";
            cin >> key;
            auto result = map.remove(key);
            if (result.has_value()) {
                cout << "Key \"" << key << "\" with value \"" << result.value() << "\" successfully removed.\n";
            }
            else {
                cout << "Key \"" << key << "\" not found.\n";
            }
            break;
        }
        case 4: {
            cout << "Key-Value pairs in the map:\n";
            for (auto it = map.begin(); it != map.end(); ++it) {
                cout << "Key: " << it->first << ", Value: " << it->second << "\n";
            }
            break;
        }
        case 5: {
            cout << "Number of elements in the map: " << map.getElementsCount() << "\n";
            break;
        }
        case 6: {
            if (map.getElementsCount() == 0) {
                cout << "The map is empty.\n";
            }
            else {
                cout << "The map is not empty.\n";
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