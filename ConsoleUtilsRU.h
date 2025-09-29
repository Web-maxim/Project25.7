// ConsoleUtilsRU.h
#pragma once
#include <string>
using namespace std;

wstring readUTF8FromConsoleRU();
string wstring_to_utf8(const wstring& str);
wstring utf8_to_wstring(const string& str);
