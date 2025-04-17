#include <iostream>
#include <memory>
#include <string>
#include "Heap.h" 

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
        << " 1 to add an element to the Heap\n"
        << " 2 to pop an element from the Heap\n"
        << " 3 to display the top element of the Heap\n"
        << " 4 to check if the Heap is empty\n"
        << " 5 to check the type of the Heap (Ascending/Descending)\n"
        << " 6 to visualize the heap\n"
        << " 7 to exit\n";
}

template<typename T>
void processHeapOperations(std::unique_ptr<Heap<T>>& heap) {
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
            cout << "Enter a value to add to the Heap: ";
            cin >> value;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            heap->push(value);
            cout << "Element " << value << " added to the Heap.\n";
            break;

        case 2:
            if (!heap->empty()) {
                cout << "Element with highest priority: " << heap->pop() << "\n";
            }
            else {
                cout << "Heap is empty!\n";
            }
            break;

        case 3:
            if (!heap->empty()) {
                cout << "Top element of the Heap: " << heap->top() << "\n";
            }
            else {
                cout << "Heap is empty!\n";
            }
            break;

        case 4:
            cout << (heap->empty() ? "Heap is empty.\n" : "Heap is not empty.\n");
            break;

        case 5:
            cout << "The type of the Heap is: " << (dynamic_cast<MinHeap<T>*>(heap.get()) ? "MinHeap (Ascending)" : "MaxHeap (Descending)") << "\n";
            break;

        case 6:
            heap->visualize();
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
    int heapType = -1;

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

    cout << "Choose the type of Heap:\n"
        << "1 for MinHeap (Ascending)\n"
        << "2 for MaxHeap (Descending)\n"
        << "Enter your choice : ";
    while (heapType != 1 && heapType != 2) {
        cin >> input;
        if (isNumber(input)) {
            heapType = stoi(input);
            if (heapType != 1 && heapType != 2) {
                cout << "Invalid choice. Please choose 1 or 2 : \n";
            }
        }
        else {
            cout << "Invalid input. Please enter a number (1 or 2) : \n";
        }
    }

    if (dataType == 1) {
        if (heapType == 1) {
            unique_ptr<Heap<int>> heap = make_unique<MinHeap<int>>();
            processHeapOperations(heap);
        }
        else {
            unique_ptr<Heap<int>> heap = make_unique<MaxHeap<int>>();
            processHeapOperations(heap);
        }
    }
    else if (dataType == 2) {
        if (heapType == 1) {
            unique_ptr<Heap<double>> heap = make_unique<MinHeap<double>>();
            processHeapOperations(heap);
        }
        else {
            unique_ptr<Heap<double>> heap = make_unique<MaxHeap<double>>();
            processHeapOperations(heap);
        }
    }
    else if (dataType == 3) {
        if (heapType == 1) {
            unique_ptr<Heap<float>> heap = make_unique<MinHeap<float>>();
            processHeapOperations(heap);
        }
        else {
            unique_ptr<Heap<float>> heap = make_unique<MaxHeap<float>>();
            processHeapOperations(heap);
        }
    }
    else if (dataType == 4) {
        if (heapType == 1) {
            unique_ptr<Heap<char>> heap = make_unique<MinHeap<char>>();
            processHeapOperations(heap);
        }
        else {
            unique_ptr<Heap<char>> heap = make_unique<MaxHeap<char>>();
            processHeapOperations(heap);
        }
    }
    else if (dataType == 5) {
        if (heapType == 1) {
            unique_ptr<Heap<string>> heap = make_unique<MinHeap<string>>();
            processHeapOperations(heap);
        }
        else {
            unique_ptr<Heap<string>> heap = make_unique<MaxHeap<string>>();
            processHeapOperations(heap);
        }
    }

    cout << "Exiting program.\n";
    return 0;
}
