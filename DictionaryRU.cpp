// DictionaryRU.cpp
#include "DictionaryRU.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
using namespace std;

void DictionaryRU::loadFromFile(const string& filename) {
    ifstream file(filename); // текстовый режим вместо ios::binary
    if (!file.is_open()) {
        wcerr << L"Ошибка открытия файла: " << utf8_to_wstring(filename) << endl;
        return;
    }

    string line;
    while (getline(file, line)) {
        istringstream iss(line);
        string token;
        while (iss >> token) {
            trie.insert(utf8_to_wstring(token));
        }
    }
    file.close();


}

vector<string> DictionaryRU::getSuggestions(const string& prefix) {
    auto ws = trie.autocomplete(utf8_to_wstring(prefix));
    vector<string> out;
    out.reserve(ws.size());
    for (const auto& w : ws) out.push_back(wstring_to_utf8(w));
    return out;
}

bool DictionaryRU::contains(const string& word) {    
    return trie.search(utf8_to_wstring(word));  // Используем прямой поиск в Trie 
}