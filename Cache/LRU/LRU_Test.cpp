#include "LRU.hpp"
#include <iostream>
#include <optional>
#include <string>

int main() {
    size_t capacity = 10; 
    LRU<std::string, double> cache(capacity);

    cache.put("A", 1);
    cache.put("B", 2);
    cache.put("C", 3);
    cache.put("D", 4);
    cache.put("E", 5);
    cache.put("F", 6);
    cache.put("G", 7);
    cache.put("H", 8);
    cache.put("I", 9);
    cache.put("J", 10);
    cache.put("K", 11);

    std::string keys[] = { "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K"};
    for (const auto& key : keys) {
        std::optional<double> val = cache.get(key);
        if (val.has_value()) {
            std::cout << "Key " << key << " => " << val.value() << "\n";
        }
        else {
            std::cout << "Key " << key << " not found in cache\n";
        }
    }
    return 0;
}
