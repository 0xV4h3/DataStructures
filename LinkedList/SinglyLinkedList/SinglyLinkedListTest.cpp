#include "SinglyLinkedList.h"
#include <iostream>

int main() {
    SinglyLinkedList<std::string, double> list;
    list.push_back("Jack", 21);
    std::cout << "Added Jack\n";

    list.push_back("Mark", 20);
    std::cout << "Inserted Mark\n";

    list.insert(2, "Joe", 19);

    std::cout << "Size: " << list.size() << "\n";
    std::cout << "I am " << list[0].first << " and I am " << list[0].second << " years old.\n";
    std::cout << "My brother " << list[1].first << " is " << list[1].second << " years old.\n";
    std::cout << "My brother " << list.at(2).first << " is " << list.back().second << " years old.\n";

    return 0;
}
