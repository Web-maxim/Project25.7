// ConsoleUtilsRU.cpp
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#include "ConsoleUtilsRU.h"
#include <iostream>
#include <codecvt>
#include <locale>
#include <conio.h>
using namespace std;

wstring readUTF8FromConsoleRU() {
    wstring result;
    int ch;
    while ((ch = _getch()) != '\r') {
        if (ch == '\t') {
            result.push_back(L'\t');
            break;
        }
        else if (ch == 8) {
            if (!result.empty()) {
                result.pop_back();
                wcout << L"\b \b";
            }
        }
        else if (ch < 0x80) {
            result.push_back(static_cast<wchar_t>(ch));
            wcout << static_cast<wchar_t>(ch);
        }
        else {
            wchar_t wch = 0;
            int bytes = 0;

            if ((ch & 0xE0) == 0xC0) bytes = 2;
            else if ((ch & 0xF0) == 0xE0) bytes = 3;

            if (bytes > 0) {
                wch = (ch & (0xFF >> (bytes + 1)));
                for (int i = 1; i < bytes; ++i) {
                    ch = _getch();
                    if ((ch & 0xC0) != 0x80) break;
                    wch = (wch << 6) | (ch & 0x3F);
                }
                result.push_back(wch);
                wcout << wch;
            }
        }
    }
    return result;
}

string wstring_to_utf8(const wstring& str) {
    wstring_convert<codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(str);
}

wstring utf8_to_wstring(const string& str) {
    wstring_convert<codecvt_utf8<wchar_t>> converter;
    return converter.from_bytes(str);
}
