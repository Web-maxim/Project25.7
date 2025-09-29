/*
// db_test.cpp
#include <iostream>
#include "sqlite3.h"
#include <Windows.h>
using namespace std;

int main_disabled() {
    // ⚙️ Настройка кодировки
    setlocale(LC_ALL, "Russian");
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    sqlite3* db;
    char* errMsg = nullptr;

    // Открываем или создаём chat.db
    if (sqlite3_open("chat.db", &db) != SQLITE_OK) {
        cerr << "Ошибка открытия БД: " << sqlite3_errmsg(db) << endl;
        return 1;
    }
    cout << "База данных открыта!" << endl;

    // Создаём таблицы
    const char* createUsers =
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "login TEXT UNIQUE, "
        "password TEXT, "
        "name TEXT);";

    const char* createMessages =
        "CREATE TABLE IF NOT EXISTS messages ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "sender TEXT, "
        "recipient TEXT, "   // NULL = всем
        "text TEXT);";

    if (sqlite3_exec(db, createUsers, 0, 0, &errMsg) != SQLITE_OK) {
        cerr << "Ошибка SQL (users): " << errMsg << endl;
        sqlite3_free(errMsg);
    }
    if (sqlite3_exec(db, createMessages, 0, 0, &errMsg) != SQLITE_OK) {
        cerr << "Ошибка SQL (messages): " << errMsg << endl;
        sqlite3_free(errMsg);
    }
    cout << "Таблицы проверены/созданы." << endl;

    // Вставляем тестовые данные
    const char* insertUser =
        "INSERT OR IGNORE INTO users (login, password, name) "
        "VALUES ('user1', 'pass1', 'Иван');";

    if (sqlite3_exec(db, insertUser, 0, 0, &errMsg) != SQLITE_OK) {
        cerr << "Ошибка SQL (insert user): " << errMsg << endl;
        sqlite3_free(errMsg);
    }

    const char* insertMsg =
        "INSERT INTO messages (sender, recipient, text) "
        "VALUES ('user1', NULL, 'Привет всем!');";

    if (sqlite3_exec(db, insertMsg, 0, 0, &errMsg) != SQLITE_OK) {
        cerr << "Ошибка SQL (insert message): " << errMsg << endl;
        sqlite3_free(errMsg);
    }

    cout << "Тестовые данные добавлены." << endl;

    // Читаем все сообщения
    const char* selectSQL = "SELECT id, sender, recipient, text FROM messages;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, selectSQL, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int id = sqlite3_column_int(stmt, 0);
            const unsigned char* sender = sqlite3_column_text(stmt, 1);
            const unsigned char* recipient = sqlite3_column_text(stmt, 2);
            const unsigned char* text = sqlite3_column_text(stmt, 3);

            cout << "[" << id << "] "
                << (sender ? (const char*)sender : "NULL") << " -> "
                << (recipient ? (const char*)recipient : "ALL") << ": "
                << (text ? (const char*)text : "") << endl;
        }
        sqlite3_finalize(stmt);
    }

    sqlite3_close(db);
    return 0;
}
*/