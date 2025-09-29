// program.cpp
#include <iostream>
#include <map>
#include "Chat.h"
#include "program.h"
#include "Database.h"
using namespace std;

program::program(const map<string, string>& cfg) : config(cfg) {}

void program::prog() {
    Database db("chat.db");
    if (!db.init()) {
        cerr << "Ошибка инициализации базы данных!" << endl;
        return;
    }

    Chat chat(db);
    chat.insert_lib();
    chat.insertRUlib(config.at("dictionary"));

    cout << "Добро пожаловать в наш чат!" << endl;

    int exitFlag = 0;
    do {
        cout << "Нажмите 1 для регистрации" << endl;
        cout << "Нажмите 2 для входа" << endl;
        cout << "Для выхода нажмите 0" << endl;

        int choice;
        cin >> choice;

        switch (choice) {
        case 1: {
            string name, login, password;
            cout << "Введите имя: ";       cin >> name;
            cout << "Введите логин: ";     cin >> login;
            cout << "Введите пароль: ";    cin >> password;
            chat.reg(name, login, password);
            break;
        }
        case 2: {
            string login, password;
            cout << "Введите логин: ";  cin >> login;
            cout << "Введите пароль: "; cin >> password;
            if (chat.log(login, password)) {
                int option;
                do {
                    cout << endl
                        << "Выберите:" << endl
                        << "1 - отправить сообщение" << endl
                        << "2 - просмотреть сообщения" << endl
                        << "3 - добавить друга" << endl
                        << "4 - выйти из аккаунта" << endl;
                    cin >> option;
                    switch (option) {
                    case 1: {
                        string message, recipient;
                        cout << "Введите сообщение: ";
                        message = chat.T9();
                        int scope = 2;
                        cout << "Кому отправить сообщение (1 - личное, 2 - всем): ";
                        cin >> scope;
                        if (scope == 2) {
                            chat.sendMessage(login, message);
                        }
                        else if (scope == 1) {
                            cout << "Введите логин получателя: ";
                            cin >> recipient;
                            chat.sendMessage(login, message, recipient);
                        }
                        break;
                    }
                    case 2:
                        chat.viewMessages(login);
                        break;
                    case 3:
                        chat.addFriend(login);
                        break;
                    case 4:
                        chat.logoutUser(login);
                        break;
                    }
                } while (option != 4);
            }
            break;
        }
        case 0:
            exitFlag = 1;
            break;
        default:
            cout << "Неверный выбор! Попробуйте ещё раз." << endl;
        }
    } while (exitFlag == 0);

    cout << "До свидания!" << endl;
}
