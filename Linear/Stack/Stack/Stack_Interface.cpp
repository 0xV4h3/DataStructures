#include "Stack.hpp"
#include <iostream>
#include <string>
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
        << " 1 to push a value onto the stack\n"
        << " 2 to pop a value from the stack\n"
        << " 3 to report the top value\n"
        << " 4 to check if the stack is empty\n"
        << " 5 to report the size of the stack\n"
        << " 6 to clear the stack\n"
        << " 7 to exit the program\n";
}

const size_t CHOICE = 7;

int main() {
    Stack<int> myStack;
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
            cout << "Enter value to push onto the stack: ";
            cin >> input;
            while (!isNumber(input)) {
                cout << "Value must be a number. Please enter a valid value: ";
                cin >> input;
            }
            int value = std::stoi(input);
            myStack.Push(value);
            cout << "Value " << value << " successfully pushed onto the stack.\n";
            break;
        }
        case 2: {
            auto poppedValue = myStack.Pop();
            if (poppedValue.has_value()) {
                cout << "Popped value from the stack: " << poppedValue.value() << "\n";
            }
            else {
                cout << "The stack is empty, cannot pop a value.\n";
            }
            break;
        }
        case 3: {
            auto toppedValue = myStack.Top();
            if (toppedValue.has_value()) {
                cout << "Top value on the stack: " << toppedValue.value() << "\n";
            }
            else {
                cout << "The stack is empty, no top value available.\n";
            }
            break;
        }
        case 4: {
            if (myStack.empty()) {
                cout << "The stack is empty.\n";
            }
            else {
                cout << "The stack is not empty.\n";
            }
            break;
        }
        case 5: {
            cout << "Size of the stack: " << myStack.size() << "\n";
            break;
        }
        case 6: {
            while (!myStack.empty()) {
                myStack.Pop();
            }
            cout << "The stack has been cleared.\n";
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
