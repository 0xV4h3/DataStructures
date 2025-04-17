#include "Trie.hpp"
#include <iostream>
#include <string>
#include <algorithm>

using namespace std;

void instructions() {
    cout << "Enter one of the following commands:\n"
        << " 1 to insert a word\n"
        << " 2 to search for a word\n"
        << " 3 to check if a prefix exists\n"
        << " 4 to remove a word\n"
        << " 5 to display all words\n"
        << " 6 to autocomplete a prefix\n"
        << " 7 to get the number of words\n"
        << " 8 to check if the Trie is empty\n"
        << " 9 to exit the program\n";
}

const size_t CHOICE = 9;

int main() {
    Trie trie;
    string input;
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
            cout << "Enter word to insert: ";
            cin >> input;
            if (trie.insert(input)) {
                cout << "Word \"" << input << "\" successfully inserted.\n";
            }
            else {
                cout << "Word \"" << input << "\" already exists.\n";
            }
            break;
        }
        case 2: {
            cout << "Enter word to search: ";
            cin >> input;
            if (trie.search(input)) {
                cout << "Word \"" << input << "\" found in the Trie.\n";
            }
            else {
                cout << "Word \"" << input << "\" not found.\n";
            }
            break;
        }
        case 3: {
            cout << "Enter prefix to check: ";
            cin >> input;
            if (trie.startsWith(input)) {
                cout << "Prefix \"" << input << "\" exists in the Trie.\n";
            }
            else {
                cout << "Prefix \"" << input << "\" does not exist.\n";
            }
            break;
        }
        case 4: {
            cout << "Enter word to remove: ";
            cin >> input;
            if (trie.remove(input)) {
                cout << "Word \"" << input << "\" successfully removed.\n";
            }
            else {
                cout << "Word \"" << input << "\" not found.\n";
            }
            break;
        }
        case 5: {
            auto words = trie.collectAllWords();
            if (words.empty()) {
                cout << "The Trie is empty.\n";
            }
            else {
                cout << "Words in the Trie:\n";
                for (const auto& word : words) {
                    cout << word << "\n";
                }
            }
            break;
        }
        case 6: {
            cout << "Enter prefix for autocomplete: ";
            cin >> input;
            auto suggestions = trie.autocomplete(input);
            if (suggestions.empty()) {
                cout << "No words found with prefix \"" << input << "\".\n";
            }
            else {
                cout << "Autocomplete suggestions:\n";
                for (const auto& word : suggestions) {
                    cout << word << "\n";
                }
            }
            break;
        }
        case 7: {
            cout << "Number of words in the Trie: " << trie.size() << "\n";
            break;
        }
        case 8: {
            if (trie.empty()) {
                cout << "The Trie is empty.\n";
            }
            else {
                cout << "The Trie is not empty.\n";
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