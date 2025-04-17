#include "Queue.hpp"
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
        << " 1 to enqueue a value into the queue\n"
        << " 2 to dequeue a value from the queue\n"
        << " 3 to check if the queue is empty\n"
        << " 4 to report the size of the queue\n"
        << " 5 to clear the queue\n"
        << " 6 to exit the program\n";
}

const size_t CHOICE = 6;

int main() {
    Queue<int> myQueue;
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
            cout << "Enter value to enqueue into the queue: ";
            cin >> input;
            while (!isNumber(input)) {
                cout << "Value must be a number. Please enter a valid value: ";
                cin >> input;
            }
            int value = std::stoi(input);
            myQueue.enqueue(value);
            cout << "Value " << value << " successfully enqueued into the queue.\n";
            break;
        }
        case 2: {
            auto dequeuedValue = myQueue.dequeue();
            if (dequeuedValue.has_value()) {
                cout << "Dequeued value from the queue: " << dequeuedValue.value().get() << "\n";
            }
            else {
                cout << "The queue is empty, cannot dequeue a value.\n";
            }
            break;
        }
        case 3: {
            if (myQueue.empty()) {
                cout << "The queue is empty.\n";
            }
            else {
                cout << "The queue is not empty.\n";
            }
            break;
        }
        case 4: {
            cout << "Size of the queue: " << myQueue.size() << "\n";
            break;
        }
        case 5: {
            myQueue.clear();
            cout << "The queue has been cleared.\n";
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
