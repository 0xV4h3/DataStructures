#pragma once

#include <memory>
#include <unordered_map>
#include <string>
#include <vector>
#include <shared_mutex>
#include <atomic>

class Trie {
private:
    struct TrieNode {
        std::unordered_map<char, std::unique_ptr<TrieNode>> children;
        bool isEndOfWord = false;
    };

    std::unique_ptr<TrieNode> _root;
    std::atomic<size_t> _wordCount;
    mutable std::shared_mutex _mutex;

    bool removeHelperImpl(TrieNode* node, const std::string& key, size_t depth);
    void collectAllWordsImpl(TrieNode* node, std::string& current, std::vector<std::string>& out) const;
    std::unique_ptr<TrieNode> cloneNode(const TrieNode* src) const;

public:
    Trie();
    Trie(const Trie& other);
    Trie(Trie&& other) noexcept;
    Trie& operator=(const Trie& other);
    Trie& operator=(Trie&& other) noexcept;
    ~Trie() = default;

    bool insert(const std::string& key);
    bool search(const std::string& key) const;
    bool startsWith(const std::string& prefix) const;
    bool remove(const std::string& key);

    std::vector<std::string> autocomplete(const std::string& prefix) const;
    std::vector<std::string> collectAllWords() const;

    size_t size() const;
    bool empty() const;
};