// program.h
#pragma once
#include <string>
#include <map>
using namespace std;

class program {
public:
    program(const map<string, string>& cfg);  
    void prog();

private:
    string dictionaryFile;   // словарь для T9
    string usersFile;        // файл пользователей
    string messagesFile;     // файл сообщений
    map<string, string> config; // сохраняем весь config 
};
