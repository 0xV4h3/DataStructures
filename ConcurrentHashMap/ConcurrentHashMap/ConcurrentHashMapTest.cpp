#include "ConcurrentHashMap.h"
#include <iostream>

int main()
{
    ConcurrentHashMap<std::string, double> Map;
    
    std::cout << "\n" << Map.getBucketCount() << std::endl;

    Map.insert("Jack", 21);
    Map.insert("Joe", 20);
    Map.insert("John", 10);

    if (Map.search("Joe"))
    {
        std::cout << Map.search("Joe").value() << std::endl;
    }

    std::cout << Map.getElementsCount();
}

