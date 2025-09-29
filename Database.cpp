// Database.cpp
#include "Database.h"
#include <iostream>
#include <sstream>
#include <cctype>
#include <algorithm>
#include "sha1.h"  

using namespace std;

// ---- helpers: SHA1 как "A,B,C,D,E" и распознавание такого формата ----
static string sha1_csv5(const string& s) {
    uint* d = sha1(s, static_cast<uint>(s.size()));
    ostringstream oss;
    oss << d[0] << "," << d[1] << "," << d[2] << "," << d[3] << "," << d[4];
    delete[] d;
    return oss.str();
}

static bool looks_like_csv5(const string& s) {
    // очень простая эвристика: 4 запятые и все символы только [0-9,]
    int commas = 0;
    for (char c : s) {
        if (c == ',') ++commas;
        else if (!isdigit(static_cast<unsigned char>(c))) return false;
    }
    return commas == 4;
}

// ----------------------------------------------------------------------

Database::Database(const string& filename) {
    if (sqlite3_open(filename.c_str(), &db) != SQLITE_OK) {
        cerr << "Ошибка открытия БД: " << sqlite3_errmsg(db) << endl;
        db = nullptr;
    }
}

Database::~Database() {
    if (db) {
        sqlite3_close(db);
    }
}

bool Database::init() {
    if (!db) return false;
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
        "recipient TEXT, "
        "text TEXT);";

    char* errMsg = nullptr;
    if (sqlite3_exec(db, createUsers, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        cerr << "Ошибка SQL (users): " << errMsg << endl;
        sqlite3_free(errMsg);
        return false;
    }
    if (sqlite3_exec(db, createMessages, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        cerr << "Ошибка SQL (messages): " << errMsg << endl;
        sqlite3_free(errMsg);
        return false;
    }
    cout << "База готова.\n";
    return true;
}

bool Database::addUser(const string& login, const string& password, const string& name) {
    if (!db) return false;

    // Храним уже ХЕШ, а не открытый пароль
    const string pass_hash = sha1_csv5(password);

    const char* sql = "INSERT OR IGNORE INTO users (login, password, name) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        cerr << "Ошибка подготовки запроса addUser\n";
        return false;
    }
    sqlite3_bind_text(stmt, 1, login.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, pass_hash.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, name.c_str(), -1, SQLITE_STATIC);

    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return ok;
}

bool Database::checkUser(const string& login, const string& password) {
    if (!db) return false;

    // 1) читаем, что лежит в поле password
    const char* sql = "SELECT password FROM users WHERE login=?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        cerr << "Ошибка подготовки запроса checkUser\n";
        return false;
    }
    sqlite3_bind_text(stmt, 1, login.c_str(), -1, SQLITE_STATIC);

    string stored;
    bool found = false;

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* p = sqlite3_column_text(stmt, 0);
        stored = p ? reinterpret_cast<const char*>(p) : "";
        found = true;
    }
    sqlite3_finalize(stmt);

    if (!found) return false;

    // 2) сравниваем
    const string candidate_hash = sha1_csv5(password);

    if (looks_like_csv5(stored)) {
        // уже хранится хеш
        return (stored == candidate_hash);
    }
    else {
        // видимо, старый пользователь с открытым паролем → мигрируем на хеш
        if (stored == password) {
            const char* upd = "UPDATE users SET password=? WHERE login=?;";
            if (sqlite3_prepare_v2(db, upd, -1, &stmt, nullptr) == SQLITE_OK) {
                sqlite3_bind_text(stmt, 1, candidate_hash.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 2, login.c_str(), -1, SQLITE_STATIC);
                sqlite3_step(stmt);
                sqlite3_finalize(stmt);
            }
            return true;
        }
        return false;
    }
}

bool Database::addMessage(const string& sender, const string& recipient, const string& text) {
    if (!db) return false;
    const char* sql = "INSERT INTO messages (sender, recipient, text) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        cerr << "Ошибка подготовки запроса addMessage\n";
        return false;
    }
    sqlite3_bind_text(stmt, 1, sender.c_str(), -1, SQLITE_STATIC);
    if (recipient.empty()) {
        sqlite3_bind_null(stmt, 2);
    }
    else {
        sqlite3_bind_text(stmt, 2, recipient.c_str(), -1, SQLITE_STATIC);
    }
    sqlite3_bind_text(stmt, 3, text.c_str(), -1, SQLITE_TRANSIENT);

    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return ok;
}

vector<Message> Database::getAllMessages() {
    vector<Message> result;
    if (!db) return result;

    const char* sql = "SELECT id, sender, recipient, text FROM messages;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            Message m;
            m.id = sqlite3_column_int(stmt, 0);

            // безопасно читаем sender
            const unsigned char* s = sqlite3_column_text(stmt, 1);
            m.sender = s ? reinterpret_cast<const char*>(s) : "";

            // безопасно читаем recipient (у тебя уже было ок)
            const unsigned char* r = sqlite3_column_text(stmt, 2);
            m.recipient = r ? reinterpret_cast<const char*>(r) : "";

            // безопасно читаем text
            const unsigned char* t = sqlite3_column_text(stmt, 3);
            m.text = t ? reinterpret_cast<const char*>(t) : "";

            result.push_back(m);
        }
        sqlite3_finalize(stmt);
    }
    return result;
}

void Database::printAllMessages() {
    for (const auto& m : getAllMessages()) {
        cout << "[" << m.id << "] " << m.sender << " -> "
            << (m.recipient.empty() ? "ALL" : m.recipient) << ": "
            << m.text << endl;
    }
} // ← закрываем функцию тут!

vector<string> Database::getAllUsers() {
    vector<string> users;
    if (!db) return users;

    const char* sql = "SELECT login FROM users ORDER BY login;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const unsigned char* login = sqlite3_column_text(stmt, 0);
            if (login) users.emplace_back(reinterpret_cast<const char*>(login));
        }
        sqlite3_finalize(stmt);
    }
    else {
        cerr << "Ошибка подготовки запроса getAllUsers\n";
    }

    return users;
}

