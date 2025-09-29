// DictionaryRU.h
#pragma once
#include <vector>
#include <string>
#include "Trie.h"
#include "ConsoleUtilsRU.h"
using namespace std;

class DictionaryRU {
private:
    Trie trie;

public:
    void loadFromFile(const string& filename);
    vector<string> getSuggestions(const string& prefix);
    bool contains(const string& word);
};
