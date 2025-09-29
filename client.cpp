// client.cpp
#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <vector>
#include "Config.h"   // читать ip/port из config.txt

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#define INVALID_SOCKET -1
#define SOCKET_ERROR   -1
typedef int SOCKET;
#endif

using namespace std;

static atomic<bool> running(true);

// надёжная отправка всего буфера (боремся с частичной отправкой)
static bool sendAll(SOCKET s, const char* data, int len) {
    int sent = 0;
    while (sent < len) {
        int rc = send(s, data + sent, len - sent, 0);
        if (rc == SOCKET_ERROR || rc == 0) return false;
        sent += rc;
    }
    return true;
}

// поток приёма сообщений (построчный парсинг + блок [USERS])
static void receiveLoop(SOCKET sock) {
    string acc;                 // аккумулятор «хвостов» между recv
    bool inUsers = false;       // внутри блока [USERS]..[END]
    vector<string> users;       // временный сбор пользователей

    char buf[1024];
    while (running) {
        int n = recv(sock, buf, sizeof(buf), 0);
        if (n > 0) {
            acc.append(buf, buf + n);

            // вынимаем полные строки по '\n'
            size_t pos;
            while ((pos = acc.find('\n')) != string::npos) {
                string line = acc.substr(0, pos);
                acc.erase(0, pos + 1);

                // уберём CR, если пришёл "\r\n"
                if (!line.empty() && line.back() == '\r') line.pop_back();

                // обработка спец-блока [USERS]
                if (!inUsers && line == "[USERS]") {
                    inUsers = true;
                    users.clear();
                    continue;
                }
                if (inUsers) {
                    if (line == "[END]") {
                        inUsers = false;
                        // выводим список пользователей
                        cout << "\n=== Пользователи (" << users.size() << ") ===\n";
                        for (const auto& u : users) cout << " - " << u << '\n';
                        cout << "= = = = = = = = = = = = = = = =\n> ";
                        cout.flush();
                        continue;
                    }
                    else {
                        if (!line.empty()) users.push_back(line);
                        continue;
                    }
                }

                // обычная строка (сообщение сервера/история/чаты)
                if (!line.empty()) {
                    cout << "\n" << line << "\n> ";
                    cout.flush();
                }
            }
        }
        else if (n == 0) {
            cout << "\n[Сервер отключился]\n";
            running = false;
            break;
        }
        else {
            // recv вернул ошибку — выходим из цикла
            cout << "\n[Ошибка приёма]\n";
            running = false;
            break;
        }
    }
}

int client_main() {
#ifdef _WIN32
    // консоль в UTF-8 для корректной кириллицы
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        cerr << "Ошибка инициализации Winsock\n";
        return 1;
    }
#endif

    // читаем ip/port из config.txt
    auto cfg = loadConfig("config.txt");
    string ip = "127.0.0.1";
    int port = 5000;
    try { ip = cfg.at("ip"); }
    catch (...) {}
    try { port = stoi(cfg.at("port")); }
    catch (...) {}

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        cerr << "Ошибка создания сокета\n";
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(static_cast<uint16_t>(port));

    if (
#ifdef _WIN32
        InetPtonA(AF_INET, ip.c_str(), &serverAddr.sin_addr)
#else
        inet_pton(AF_INET, ip.c_str(), &serverAddr.sin_addr)
#endif
        != 1) {
        cerr << "inet_pton: некорректный IP-адрес\n";
#ifdef _WIN32
        closesocket(sock);
        WSACleanup();
#else
        close(sock);
#endif
        return 1;
    }

    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "Не удалось подключиться к серверу " << ip << ":" << port << "\n";
#ifdef _WIN32
        closesocket(sock);
        WSACleanup();
#else
        close(sock);
#endif
        return 1;
    }

    // логин/пароль
    string login, password;
    cout << "Введите ваш логин: ";
    getline(cin, login);
    cout << "Введите пароль: ";
    getline(cin, password);

    // отправляем с завершающим \n, чтобы сервер мог безопасно прочитать "линию"
    string authData = login + ":" + password + "\n";
    if (!sendAll(sock, authData.c_str(), (int)authData.size())) {
        cerr << "Ошибка отправки авторизационных данных\n";
#ifdef _WIN32
        closesocket(sock);
        WSACleanup();
#else
        close(sock);
#endif
        return 1;
    }

    // ждём ответ (OK/FAIL) — читаем ровно до '\n'
    string reply;
    char ch;
    while (true) {
        int n = recv(sock, &ch, 1, 0);
        if (n <= 0) {
            cerr << "Ошибка: сервер не ответил\n";
#ifdef _WIN32
            closesocket(sock);
            WSACleanup();
#else
            close(sock);
#endif
            return 1;
        }
        if (ch == '\n') break;
        if (ch != '\r') reply.push_back(ch);
    }

    if (reply.find("FAIL") != string::npos) {
        cerr << "Авторизация не удалась!\n";
#ifdef _WIN32
        closesocket(sock);
        WSACleanup();
#else
        close(sock);
#endif
        return 1;
    }

    cout << "Авторизация успешна!\n";
    cout << "=== История чата ===\n";

    // запуск приёмника
    thread receiver(receiveLoop, sock);

    cout << "Теперь можно писать сообщения (exit для выхода):\n";

    string msg;
    while (running) {
        cout << "> ";
        if (!getline(cin, msg)) break;
        if (msg == "exit") {
            running = false;
            // аккуратно закрываем запись, чтобы поток приёма вышел
#ifdef _WIN32
            shutdown(sock, SD_SEND);
#else
            shutdown(sock, SHUT_WR);
#endif
            break;
        }

        // отправляем каждое сообщение с '\n' (протокол «построчно»)
        msg.push_back('\n');
        if (!sendAll(sock, msg.c_str(), (int)msg.size())) {
            cerr << "\nОшибка отправки. Соединение разорвано?\n";
            running = false;
            break;
        }
    }

    if (receiver.joinable()) receiver.join();

#ifdef _WIN32
    closesocket(sock);
    WSACleanup();
#else
    close(sock);
#endif
    return 0;
}
