// Trie.h
#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <memory>
using namespace std;

class Trie {
private:
    bool isEndOfWord;
    map<wchar_t, unique_ptr<Trie>> children;

    void autocompleteHelper(const wstring& prefix, vector<wstring>& results) const;

public:
    Trie();

    void insert(const wstring& word);
    bool search(const wstring& word) const;
    vector<wstring> autocomplete(const wstring& prefix) const;
};
