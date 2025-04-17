#include "Trie.hpp"

Trie::Trie()
    : _root(std::make_unique<TrieNode>()), _wordCount(0) {
}

Trie::Trie(const Trie& other)
    : _root(cloneNode(other._root.get())), _wordCount(other._wordCount.load()) {
}

Trie::Trie(Trie&& other) noexcept
    : _root(std::move(other._root)), _wordCount(other._wordCount.load()) {
}

Trie& Trie::operator=(const Trie& other) {
    if (this != &other) {
        std::unique_lock lock(_mutex);
        std::shared_lock otherLock(other._mutex);
        _root = cloneNode(other._root.get());
        _wordCount.store(other._wordCount.load());
    }
    return *this;
}

Trie& Trie::operator=(Trie&& other) noexcept {
    if (this != &other) {
        std::unique_lock lock(_mutex);
        _root = std::move(other._root);
        _wordCount.store(other._wordCount.load());
    }
    return *this;
}

bool Trie::insert(const std::string& key) {
    std::unique_lock lock(_mutex);
    TrieNode* node = _root.get();
    for (char c : key) {
        auto& child = node->children[c];
        if (!child) child = std::make_unique<TrieNode>();
        node = child.get();
    }
    if (node->isEndOfWord) return false;
    node->isEndOfWord = true;
    _wordCount++;
    return true;
}

bool Trie::search(const std::string& key) const {
    std::shared_lock lock(_mutex);
    TrieNode* node = _root.get();
    for (char c : key) {
        auto it = node->children.find(c);
        if (it == node->children.end()) return false;
        node = it->second.get();
    }
    return node->isEndOfWord;
}

bool Trie::startsWith(const std::string& prefix) const {
    std::shared_lock lock(_mutex);
    TrieNode* node = _root.get();
    for (char c : prefix) {
        auto it = node->children.find(c);
        if (it == node->children.end()) return false;
        node = it->second.get();
    }
    return true;
}

bool Trie::remove(const std::string& key) {
    std::unique_lock lock(_mutex);
    bool removed = removeHelperImpl(_root.get(), key, 0);
    if (removed) _wordCount--;
    return removed;
}

bool Trie::removeHelperImpl(TrieNode* node, const std::string& key, size_t depth) {
    if (!node) return false;
    if (depth == key.size()) {
        if (!node->isEndOfWord) return false;
        node->isEndOfWord = false;
        return node->children.empty();
    }
    char c = key[depth];
    auto it = node->children.find(c);
    if (it == node->children.end()) return false;
    bool shouldDeleteChild = removeHelperImpl(it->second.get(), key, depth + 1);
    if (shouldDeleteChild) {
        node->children.erase(c);
        return !node->isEndOfWord && node->children.empty();
    }
    return false;
}

std::vector<std::string> Trie::autocomplete(const std::string& prefix) const {
    std::shared_lock lock(_mutex);
    TrieNode* node = _root.get();
    for (char c : prefix) {
        auto it = node->children.find(c);
        if (it == node->children.end()) return {};
        node = it->second.get();
    }
    std::vector<std::string> results;
    std::string current = prefix;
    collectAllWordsImpl(node, current, results);
    return results;
}

std::vector<std::string> Trie::collectAllWords() const {
    std::shared_lock lock(_mutex);
    std::vector<std::string> results;
    std::string current;
    collectAllWordsImpl(_root.get(), current, results);
    return results;
}

void Trie::collectAllWordsImpl(TrieNode* node, std::string& current, std::vector<std::string>& out) const {
    if (node->isEndOfWord) out.push_back(current);
    for (auto& [c, child] : node->children) {
        current.push_back(c);
        collectAllWordsImpl(child.get(), current, out);
        current.pop_back();
    }
}

size_t Trie::size() const {
    return _wordCount.load();
}

bool Trie::empty() const {
    return size() == 0;
}

std::unique_ptr<Trie::TrieNode> Trie::cloneNode(const TrieNode* src) const {
    auto node = std::make_unique<TrieNode>();
    node->isEndOfWord = src->isEndOfWord;
    for (auto& [c, child] : src->children) {
        node->children[c] = cloneNode(child.get());
    }
    return node;
}