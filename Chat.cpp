// Chat.cpp
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING

#ifdef _WIN32
// Предотвращаем конфликты с std::byte и макросами Windows.
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#define _HAS_STD_BYTE 0
#endif

#include "Chat.h"
#include <iostream>
#include <locale>
#include <stdexcept>
#include <istream>   // для std::ws
#include <codecvt>   // codecvt для non-Windows и как запасной вариант

#ifdef _WIN32
#include <Windows.h>
#endif

using namespace std;

namespace {

    // --- UTF-8/CP1251 -> wstring (устойчивое преобразование) ---
    inline wstring to_wide_resilient(const string& s) {
#ifdef _WIN32
        // 1) строгий UTF-8
        int need = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s.c_str(), (int)s.size(), nullptr, 0);
        if (need > 0) {
            wstring ws(need, L'\0');
            MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s.c_str(), (int)s.size(), &ws[0], need);
            return ws;
        }
        // 2) пробуем CP1251 (наследие старых записей в БД)
        need = MultiByteToWideChar(1251, 0, s.c_str(), (int)s.size(), nullptr, 0);
        if (need > 0) {
            wstring ws(need, L'\0');
            MultiByteToWideChar(1251, 0, s.c_str(), (int)s.size(), &ws[0], need);
            return ws;
        }
        // 3) последний рубеж: ASCII с заменой
        wstring ws; ws.reserve(s.size());
        for (unsigned char c : s) ws.push_back(c < 0x80 ? wchar_t(c) : L'?');
        return ws;
#else
        try {
            wstring_convert<codecvt_utf8_utf16<wchar_t>> conv;
            return conv.from_bytes(s);
        }
        catch (...) {
            wstring ws; ws.reserve(s.size());
            for (unsigned char c : s) ws.push_back(c < 0x80 ? wchar_t(c) : L'?');
            return ws;
        }
#endif
    }

    // --- wstring -> UTF-8 ---
    inline string to_utf8_resilient(const wstring& ws) {
#ifdef _WIN32
        int need = WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), (int)ws.size(), nullptr, 0, nullptr, nullptr);
        if (need > 0) {
            string out(need, '\0');
            WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), (int)ws.size(), &out[0], need, nullptr, nullptr);
            return out;
        }
        // запасной вариант: выкинем не-ASCII
        string out; out.reserve(ws.size());
        for (wchar_t c : ws) out.push_back((c >= 0 && c < 0x80) ? char(c) : '?');
        return out;
#else
        wstring_convert<codecvt_utf8_utf16<wchar_t>> conv;
        return conv.to_bytes(ws);
#endif
    }

} // namespace

Chat::Chat(Database& database) : db(database) {}

bool Chat::log(const string& _login, const string& _pass) {
    if (db.checkUser(_login, _pass)) {
        wcout << L"Пользователь " << to_wide_resilient(_login) << L" вошёл в чат." << endl;
        return true;
    }
    wcout << L"Ошибка: неверный логин или пароль." << endl;
    return false;
}

bool Chat::reg(const string& _name, const string& _login, const string& _pass) {
    if (db.addUser(_login, _pass, _name)) {
        wcout << L"Пользователь " << to_wide_resilient(_name) << L" успешно зарегистрирован!\n" << endl;
        friends->vname.push_back(_login);
        friends->addVertex(static_cast<int>(friends->vname.size() - 1));
        return true;
    }
    wcout << L"Ошибка: логин уже занят." << endl;
    return false;
}

void Chat::insertRUlib(const string& filename) {
    dictRU->loadFromFile(filename);
}

string Chat::T9RU() {
    return to_utf8_resilient(readInputWithAutocompleteRU(*dictRU));
}

void Chat::logoutUser(const string& login) {
    wcout << L"Пользователь " << to_wide_resilient(login) << L" вышел из чата." << endl;
}

void Chat::sendMessage(const string& senderLogin, const string& message, const string& recipient) {
    if (!recipient.empty()) {
        db.addMessage(senderLogin, recipient, message);
        wcout << to_wide_resilient(senderLogin)
            << L" (лично " << to_wide_resilient(recipient)
            << L"): " << to_wide_resilient(message) << endl;
    }
    else {
        db.addMessage(senderLogin, "", message);
        wcout << to_wide_resilient(senderLogin)
            << L" (всем): " << to_wide_resilient(message) << endl;
    }
}

void Chat::listUsers(const string& login) const {
    friends->findMinDistancesFloyd(login);
}

void Chat::viewMessages(const string& login) const {
    wcout << L"Сообщения для " << to_wide_resilient(login) << L":" << endl;
    auto messages = db.getAllMessages();
    for (const auto& msg : messages) {
        if (msg.recipient.empty()) {
            wcout << to_wide_resilient(msg.sender)
                << L" (всем): " << to_wide_resilient(msg.text) << endl;
        }
        else if (msg.recipient == login || msg.sender == login) {
            wcout << to_wide_resilient(msg.sender)
                << L" (лично " << to_wide_resilient(msg.recipient) << L"): "
                << to_wide_resilient(msg.text) << endl;
        }
    }
}

void Chat::addFriend(const string& user_name) {
    wcout << L"Кого вы хотите добавить в друзья? Для отмены введите 0." << endl;
    listUsers(user_name);
    string friend_name;
    while (true) {
        wcout << L"Введите логин: ";
        cin >> friend_name;
        if (friend_name == "0") break;
        wcout << L"Добавлен в друзья: " << to_wide_resilient(friend_name) << endl;
        break;
    }
}

string Chat::T9() {
    auto wsuggestions = trie->autocomplete(L"");
    if (wsuggestions.empty()) {
        wcout << L"Словарь пуст. Введите своё слово: ";
        string input;
        getline(cin >> ws, input);            // std::ws — пропускаем пробелы/переводы строк
        trie->insert(utf8_to_wstring(input)); // объявлена в ConsoleUtilsRU.h
        return input;
    }
    return wstring_to_utf8(wsuggestions[0]);  // из ConsoleUtilsRU.h
}

void Chat::insert_lib() {
    trie->insert(utf8_to_wstring("Hello"));
    trie->insert(utf8_to_wstring("How"));
    trie->insert(utf8_to_wstring("are"));
    trie->insert(utf8_to_wstring("you"));
    trie->insert(utf8_to_wstring("hi"));
}
