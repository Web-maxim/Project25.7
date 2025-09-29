// server.cpp
#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
#define _HAS_STD_BYTE 0
#define NOMINMAX
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <cstring>      // ← для strlen
#include "Database.h"
#include "Config.h"     // для port и max_message_length

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#endif

using namespace std;

// теперь не const, чтобы можно было подхватить значение из конфига
static size_t MAX_MSG_LEN = 200;

// мапим логин -> сокет (для личных сообщений)
static unordered_map<string, SOCKET> loginToSock;

// аккуратно обрезаем пробелы/CR/LF по краям
static inline std::string trim_copy(const std::string& s) {
    const auto b = s.find_first_not_of(" \t\r\n");
    if (b == std::string::npos) return "";
    const auto e = s.find_last_not_of(" \t\r\n");
    return s.substr(b, e - b + 1);
}

// отправка списка пользователей конкретному клиенту
static void sendUsersListTo(SOCKET client, Database& db) {
    auto users = db.getAllUsers();
    string start = "[USERS]\n";
    send(client, start.c_str(), (int)start.size(), 0);
    for (const auto& u : users) {
        string line = u + "\n";
        send(client, line.c_str(), (int)line.size(), 0);
    }
    string end = "[END]\n";
    send(client, end.c_str(), (int)end.size(), 0);
}

int server_main() {
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

    // читаем конфиг (порт и лимит длины сообщений)
    auto cfg = loadConfig("config.txt");
    int port = 5000;
    try { port = stoi(cfg.at("port")); }
    catch (...) {}
    try { MAX_MSG_LEN = static_cast<size_t>(stoul(cfg.at("max_message_length"))); }
    catch (...) {}

    // БД
    Database db("chat.db");
    if (!db.init()) {
        cerr << "Ошибка инициализации базы данных!" << endl;
        return 1;
    }

    SOCKET serverSock = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSock == INVALID_SOCKET) {
        cerr << "Ошибка создания сокета!" << endl;
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(static_cast<uint16_t>(port));
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "Ошибка bind!" << endl;
#ifdef _WIN32
        closesocket(serverSock);
        WSACleanup();
#else
        close(serverSock);
#endif
        return 1;
    }

    listen(serverSock, 5);
    cout << "Сервер запущен на порту " << port << endl;

    fd_set master;
    FD_ZERO(&master);
    FD_SET(serverSock, &master);

    map<SOCKET, string> clientNames;
    unordered_map<SOCKET, string> acc; // аккумуляторы построчного приёма

    while (true) {
        fd_set copy = master;
        int socketCount = select(0, &copy, nullptr, nullptr, nullptr);

        for (int i = 0; i < socketCount; i++) {
            SOCKET sock = copy.fd_array[i];

            if (sock == serverSock) {
                // новый клиент
                SOCKET client = accept(serverSock, nullptr, nullptr);
                if (client == INVALID_SOCKET) continue;
                FD_SET(client, &master);

                bool authorized = false;
                string login = "guest", pass;

                // читаем первую строку: "login:password\n"
                string firstMsg;
                char ch;
                while (true) {
                    int m = recv(client, &ch, 1, 0);
                    if (m <= 0) { authorized = false; break; }
                    if (ch == '\n') break;
                    if (ch != '\r') firstMsg.push_back(ch);
                }

                if (!firstMsg.empty()) {
                    size_t pos = firstMsg.find(':');
                    if (pos != string::npos) {
                        login = firstMsg.substr(0, pos);
                        pass = firstMsg.substr(pos + 1);
                    }
                    else {
                        login = firstMsg;
                        pass = "nopass";
                    }

                    if (db.checkUser(login, pass)) {
                        clientNames[client] = login;
                        authorized = true;
                    }
                    else {
                        if (db.addUser(login, pass, login)) {
                            clientNames[client] = login;
                            authorized = true;
                        }
                        else {
                            authorized = false;
                        }
                    }
                }

                if (!authorized) {
                    string err = "FAIL\n";
                    send(client, err.c_str(), (int)err.size(), 0);
#ifdef _WIN32
                    closesocket(client);
#else
                    close(client);
#endif
                    FD_CLR(client, &master);
                    continue;
                }
                else {
                    string ok = "OK\n";
                    send(client, ok.c_str(), (int)ok.size(), 0);
                }

                // добавляем в мапу логинов для ЛС
                loginToSock[clientNames[client]] = client;

                // сообщение о подключении
                string msg = "[Сервер] " + clientNames[client] + " подключился\n";
                cout << msg;

                // отправляем историю (только публичное и мои приватные)
                const string& me = clientNames[client];
                auto history = db.getAllMessages();
                for (const auto& m : history) {
                    const bool isPublic = m.recipient.empty();
                    const bool iAmSender = (m.sender == me);
                    const bool iAmRecipient = (!m.recipient.empty() && m.recipient == me);
                    if (!(isPublic || iAmSender || iAmRecipient)) continue;

                    string line = "[" + m.sender +
                        (m.recipient.empty() ? " -> ALL" : " -> " + m.recipient) +
                        "] " + m.text + "\n";
                    send(client, line.c_str(), (int)line.size(), 0);
                }

                // отправляем список пользователей подключившемуся
                sendUsersListTo(client, db);

                // оповестим остальных
                for (u_int j = 0; j < master.fd_count; j++) {
                    SOCKET outSock = master.fd_array[j];
                    if (outSock != serverSock && outSock != client) {
                        send(outSock, msg.c_str(), (int)msg.size(), 0);
                    }
                }
            }
            else {
                char buffer[1024];
                int n = recv(sock, buffer, sizeof(buffer), 0);

                if (n <= 0) {
                    string name = clientNames[sock];
                    string msg = "[Сервер] " + name + " отключился\n";
                    cout << msg;

                    // чистим структуры
                    clientNames.erase(sock);
                    acc.erase(sock);
                    if (!name.empty()) {
                        auto it = loginToSock.find(name);
                        if (it != loginToSock.end() && it->second == sock) {
                            loginToSock.erase(it);
                        }
                    }

                    FD_CLR(sock, &master);
#ifdef _WIN32
                    closesocket(sock);
#else
                    close(sock);
#endif
                    // рассылаем уведомление
                    for (u_int j = 0; j < master.fd_count; j++) {
                        SOCKET outSock = master.fd_array[j];
                        if (outSock != serverSock) {
                            send(outSock, msg.c_str(), (int)msg.size(), 0);
                        }
                    }
                }
                else {
                    // построчный приём + фильтрация пустых
                    acc[sock].append(buffer, buffer + n);

                    size_t pos;
                    while ((pos = acc[sock].find('\n')) != string::npos) {
                        string line = acc[sock].substr(0, pos);
                        acc[sock].erase(0, pos + 1);
                        if (!line.empty() && line.back() == '\r') line.pop_back();

                        string text = trim_copy(line);
                        if (text.empty()) continue;

                        // /help — краткая справка
                        if (text == "/help") {
                            const char* help =
                                "[Сервер] Команды:\n"
                                "  /users              — список пользователей\n"
                                "  /w <login> <текст>  — личное сообщение\n"
                                "  exit                — выход (на клиенте)\n";
                            send(sock, help, (int)strlen(help), 0);
                            continue;
                        }

                        // /users — выдать список
                        if (text == "/users") {
                            sendUsersListTo(sock, db);
                            continue;
                        }

                        // /w <login> <текст> — личное сообщение
                        if (text.rfind("/w ", 0) == 0) {   // ← БЕЗ обратных слэшей
                            string rest = trim_copy(text.substr(3));
                            size_t sp = rest.find(' ');
                            if (sp == string::npos) {
                                string help = "[Сервер] Использование: /w <login> <текст>\n";
                                send(sock, help.c_str(), (int)help.size(), 0);
                                continue;
                            }
                            string toLogin = trim_copy(rest.substr(0, sp));
                            string body = trim_copy(rest.substr(sp + 1));
                            if (toLogin.empty() || body.empty()) {
                                string help = "[Сервер] Использование: /w <login> <текст>\n";
                                send(sock, help.c_str(), (int)help.size(), 0);
                                continue;
                            }

                            auto it = loginToSock.find(toLogin);
                            if (it == loginToSock.end()) {
                                string err = "[Сервер] Пользователь '" + toLogin + "' не в сети\n";
                                send(sock, err.c_str(), (int)err.size(), 0);
                                continue;
                            }

                            // ограничение длины
                            if (body.size() > MAX_MSG_LEN) body.resize(MAX_MSG_LEN);

                            string from = clientNames[sock];
                            string out = "[" + from + " -> " + toLogin + "] " + body + "\n";

                            // отправляем адресату и отправителю (подтверждение)
                            send(it->second, out.c_str(), (int)out.size(), 0);
                            send(sock, out.c_str(), (int)out.size(), 0);

                            // сохраняем в БД как приватное
                            db.addMessage(from, toLogin, body);
                            continue;
                        }

                        // обычное сообщение во весь чат
                        if (text.size() > MAX_MSG_LEN)
                            text.resize(MAX_MSG_LEN);

                        string out = "[" + clientNames[sock] + "] " + text + "\n";
                        cout << out;

                        db.addMessage(clientNames[sock], "", text);

                        for (u_int j = 0; j < master.fd_count; j++) {
                            SOCKET outSock = master.fd_array[j];
                            if (outSock != serverSock && outSock != sock) {
                                send(outSock, out.c_str(), (int)out.size(), 0);
                            }
                        }
                    }
                }
            }
        }
    }

#ifdef _WIN32
    closesocket(serverSock);
    WSACleanup();
#else
    close(serverSock);
#endif
    return 0;
}
