// Trie.cpp
#include "Trie.h"
using namespace std;

Trie::Trie() : isEndOfWord(false) {}

void Trie::insert(const wstring& word) {
    Trie* node = this;
    for (wchar_t ch : word) {
        if (!node->children.count(ch)) {
            node->children[ch] = make_unique<Trie>();
        }
        node = node->children[ch].get();
    }
    node->isEndOfWord = true;
}

bool Trie::search(const wstring& word) const {
    const Trie* node = this;
    for (wchar_t ch : word) {
        auto it = node->children.find(ch);
        if (it == node->children.end()) return false;
        node = it->second.get();
    }
    return node->isEndOfWord;
}

void Trie::autocompleteHelper(const wstring& prefix, vector<wstring>& results) const {
    if (isEndOfWord) {
        results.push_back(prefix);
    }
    for (const auto& [ch, child] : children) {
        child->autocompleteHelper(prefix + ch, results);
    }
}

vector<wstring> Trie::autocomplete(const wstring& prefix) const {
    const Trie* node = this;
    for (wchar_t ch : prefix) {
        auto it = node->children.find(ch);
        if (it == node->children.end()) return {};
        node = it->second.get();
    }

    vector<wstring> results;
    node->autocompleteHelper(prefix, results);
    return results;
}
