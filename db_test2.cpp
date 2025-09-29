// db_test2.cpp
#include <iostream>
#include <Windows.h>
#include "Database.h"

using namespace std;

int db_test_main() {
    // ⚙️ Настройка кодировки для русского языка
    setlocale(LC_ALL, "Russian");
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    Database db("chat.db");

    if (!db.init()) {
        cerr << "Не удалось инициализировать базу данных.\n";
        return 1;
    }

    cout << "=== Тест пользователей ===\n";
    db.addUser("user1", "pass1", "Иван");
    db.addUser("user2", "pass2", "Пётр");

    cout << "user1/pass1 -> "
        << (db.checkUser("user1", "pass1") ? "OK" : "FAIL") << endl;
    cout << "user2/wrong -> "
        << (db.checkUser("user2", "wrong") ? "OK" : "FAIL") << endl;

    cout << "\n=== Тест сообщений ===\n";
    db.addMessage("user1", "user2", "Привет, user2!");
    db.addMessage("user2", "user1", "И тебе привет, user1!");
    db.addMessage("user1", "", "Сообщение для всех!");

    auto allMessages = db.getAllMessages();
    for (const auto& m : allMessages) {
        cout << "[" << m.id << "] "
            << m.sender << " -> "
            << (m.recipient.empty() ? "ALL" : m.recipient) << ": "
            << m.text << endl;
    }

    return 0;
}