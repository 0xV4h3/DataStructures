#include <iostream>
#include <string>
#include <memory>
#include "PriorityQueue.h" 

using namespace std;

const int CHOICE = 7; 

bool isNumber(const string& str) {
    if (str.empty()) return false;
    size_t start = (str[0] == '-') ? 1 : 0;
    for (size_t i = start; i < str.length(); ++i) {
        if (!isdigit(str[i])) return false;
    }
    return true;
}

void datatype() {
    cout << "\nChoose the type of data:\n"
        << "1 for int\n"
        << "2 for double\n"
        << "3 for float\n"
        << "4 for char\n"
        << "5 for string\n";
}

void instructions() {
    cout << "\nEnter one of the following options:\n"
        << " 1 to add an element to the PriorityQueue\n"
        << " 2 to pop an element from the PriorityQueue\n"
        << " 3 to display the top element of the PriorityQueue\n"
        << " 4 to check if the PriorityQueue is empty\n"
        << " 5 to check the type of the PriorityQueue (Ascending/Descending)\n"
        << " 6 to visualize the PriorityQueue\n"
        << " 7 to exit\n";
}

template<typename T>
void processQueueOperations(unique_ptr<PriorityQueue<T>>& pq) {
    T value;
    string input;
    int choice;

    instructions();
    cout << "What do you want to do? ";

    while (true) {
        cin >> input;

        if (isNumber(input)) {
            choice = stoi(input);
            if (choice == CHOICE) break;
        }
        else {
            cout << "Invalid choice. Please enter a number between 1 and " << CHOICE << ": ";
            continue;
        }

        switch (choice) {
        case 1:
            cout << "Enter a value to add to the PriorityQueue: ";
            cin >> value;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            pq->push(value);
            cout << "Element " << value << " added to the PriorityQueue.\n";
            break;

        case 2:
            if (!pq->empty()) {
                cout << "Element with highest priority: " << pq->pop() << "\n";
            }
            else {
                cout << "PriorityQueue is empty!\n";
            }
            break;

        case 3:
            if (!pq->empty()) {
                cout << "Top element of the PriorityQueue: " << pq->top() << "\n";
            }
            else {
                cout << "PriorityQueue is empty!\n";
            }
            break;

        case 4:
            cout << (pq->empty() ? "PriorityQueue is empty.\n" : "PriorityQueue is not empty.\n");
            break;

        case 5:
            cout << "The type of the PriorityQueue is: " << pq->type() << "\n";
            break;

        case 6:
            if (!pq->empty()) {
                cout << "PriorityQueue\n";
                pq->visualize();
            }
            else {
                cout << "PriorityQueue is empty!\n";
            }
            break;
        default:
            cout << "Invalid choice. Please try again.\n";
            break;
        }

        cout << "What do you want to do next? ";
    }
}

int main() {
    string input;
    int dataType = -1;
    int queueType = -1;

    datatype();
    cout << "Enter your choice (1-5) : ";
    while (dataType < 1 || dataType > 5) {
        cin >> input;
        if (isNumber(input)) {
            dataType = stoi(input);
            if (dataType < 1 || dataType > 5) {
                cout << "Invalid choice. Please choose a number between 1 and 5 : ";
            }
        }
        else {
            cout << "Invalid input. Please enter a valid number (1-5) : ";
        }
    }

    cout << "Choose the type of PriorityQueue:\n"
        << "1 for MinHeap (Ascending Queue)\n"
        << "2 for MaxHeap (Descending Queue)\n"
        << "Enter your choice : ";
    while (queueType != 1 && queueType != 2) {
        cin >> input;
        if (isNumber(input)) {
            queueType = stoi(input);
            if (queueType != 1 && queueType != 2) {
                cout << "Invalid choice. Please choose 1 or 2 : \n";
            }
        }
        else {
            cout << "Invalid input. Please enter a number (1 or 2) : \n";
        }
    }

    if (dataType == 1) {
        unique_ptr<PriorityQueue<int>> pq = (queueType == 1) ? make_unique<PriorityQueue<int>>(new MinHeap<int>())
            : make_unique<PriorityQueue<int>>(new MaxHeap<int>());
        processQueueOperations(pq);
    }
    else if (dataType == 2) {
        unique_ptr<PriorityQueue<double>> pq = (queueType == 1) ? make_unique<PriorityQueue<double>>(new MinHeap<double>())
            : make_unique<PriorityQueue<double>>(new MaxHeap<double>());
        processQueueOperations(pq);
    }
    else if (dataType == 3) {
        unique_ptr<PriorityQueue<float>> pq = (queueType == 1) ? make_unique<PriorityQueue<float>>(new MinHeap<float>())
            : make_unique<PriorityQueue<float>>(new MaxHeap<float>());
        processQueueOperations(pq);
    }
    else if (dataType == 4) {
        unique_ptr<PriorityQueue<char>> pq = (queueType == 1) ? make_unique<PriorityQueue<char>>(new MinHeap<char>())
            : make_unique<PriorityQueue<char>>(new MaxHeap<char>());
        processQueueOperations(pq);
    }
    else if (dataType == 5) {
        unique_ptr<PriorityQueue<string>> pq = (queueType == 1) ? make_unique<PriorityQueue<string>>(new MinHeap<string>())
            : make_unique<PriorityQueue<string>>(new MaxHeap<string>());
        processQueueOperations(pq);
    }

    cout << "Exiting program.\n";
    return 0;
}
