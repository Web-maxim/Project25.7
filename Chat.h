// Chat.h
#pragma once
#include <iostream>
#include <string>
#include <memory>
#include "Database.h"
#include "Trie.h"
#include "Graph.h"
#include "AutocompleteRU.h"
#include "DictionaryRU.h"
using namespace std;

class Chat {
private:
    Database& db;  // ссылка на базу данных

public:
    Chat(Database& database);

    // словари автодополнения
    unique_ptr<DictionaryRU> dictRU = make_unique<DictionaryRU>();
    string T9RU();
    void insertRUlib(const string& filename);

    unique_ptr<Graph> friends = make_unique<Graph>();
    unique_ptr<Trie> trie = make_unique<Trie>();

    // регистрация и вход
    bool reg(const string& _name, const string& _login, const string& _pass);
    bool log(const string& _login, const string& _pass);
    void logoutUser(const string& login);

    // работа с сообщениями
    void sendMessage(const string& senderLogin, const string& message, const string& recipient = "");
    void listUsers(const string& login) const;
    void viewMessages(const string& login) const;

    // друзья
    void addFriend(const string& user_name);

    // автодополнение
    string T9();
    void insert_lib();
};
