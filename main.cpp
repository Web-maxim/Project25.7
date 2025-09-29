// main.cpp
#define NOMINMAX
#define _HAS_STD_BYTE 0
#include <Windows.h>

#include "program.h"
#include "Config.h"
#include "server.h"
#include "client.h"

#include <iostream>
#include <map>
#include <limits>
using namespace std;

int main() {
    // ⚙️ Настройка кодировки
    setlocale(LC_ALL, "Russian");
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    auto config = loadConfig("config.txt");

    while (true) {
        cout << "\nВыберите режим:" << endl;
        cout << "1 - Локальный чат" << endl;
        cout << "2 - Сервер" << endl;
        cout << "3 - Клиент" << endl;
        cout << "0 - Выход" << endl;

        int choice;
        cin >> choice;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (choice == 0) {
            cout << "До свидания!" << endl;
            break;
        }

        switch (choice) {
        case 1: {
            cout << "Запуск локального чата..." << endl;
            program start(config);
            start.prog();
            break;
        }
        case 2:
            cout << "Запуск сервера..." << endl;
            server_main();
            break;
        case 3:
            cout << "Запуск клиента..." << endl;
            client_main();
            break;
        default:
            cout << "Неверный выбор, попробуйте ещё раз." << endl;
        }
    }

    return 0;
}
