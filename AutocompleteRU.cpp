// AutocompleteRU.cpp
#include "AutocompleteRU.h"
#include "ConsoleUtilsRU.h"
#include <iostream>
using namespace std;

wstring readInputWithAutocompleteRU(DictionaryRU& dict) {
    wstring baseInput;
    wstring current;
    bool completionShown = false;
    vector<wstring> lastSuggestions;
    size_t suggestionIndex = 0;

    while (true) {
        wstring input = readUTF8FromConsoleRU();

        if (input.empty()) continue;

        if (input.back() == L'\t') {
            input.pop_back();
            if (!completionShown) {
                if (!input.empty()) {
                    baseInput = input;
                    string prefix = wstring_to_utf8(input);

                    auto suggestions = dict.getSuggestions(prefix);

                    lastSuggestions.clear();
                    for (const auto& s : suggestions) {
                        lastSuggestions.push_back(utf8_to_wstring(s));
                    }

                    if (!lastSuggestions.empty()) {
                        wcout << L"\nВарианты:\n";
                        for (size_t i = 0; i < lastSuggestions.size(); ++i) {
                            wcout << i + 1 << L". " << lastSuggestions[i] << L" ";
                            if ((i + 1) % 3 == 0) wcout << L"\n";
                        }
                        wcout << L"\n";

                        completionShown = true;
                        suggestionIndex = 0;

                        current = lastSuggestions[suggestionIndex];
                        baseInput = current.substr(0, input.length());
                        wcout << L"\r" << wstring(50, L' ') << L"\r";
                        wcout << current;
                    }
                    else {
                        wcout << L"\nНет подходящих вариантов.\n";
                        current = input;
                        wcout << L"\r" << wstring(50, L' ') << L"\r";
                        wcout << current;
                    }
                }
            }
            else {
                if (!lastSuggestions.empty()) {
                    suggestionIndex++;
                    if (suggestionIndex >= lastSuggestions.size()) {
                        suggestionIndex = 0;
                        current = baseInput;
                    }
                    else {
                        current = lastSuggestions[suggestionIndex];
                    }
                    wcout << L"\r" << wstring(50, L' ') << L"\r";
                    wcout << current;
                }
            }
        }
        else {
            current += input;
            baseInput = current;
            completionShown = false;
            lastSuggestions.clear();
            suggestionIndex = 0;
            break;
        }
    }

    return current;

}