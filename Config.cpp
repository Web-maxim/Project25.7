// Config.cpp
#include "Config.h"
#include <fstream>
#include <sstream>
#include <iostream>

map<string, string> loadConfig(const string& filename) {
    map<string, string> config;
    ifstream file(filename);

    if (!file.is_open()) {
        // если файла нет — создаём с настройками по умолчанию
        ofstream newFile(filename);
        newFile << "ip=127.0.0.1\n";
        newFile << "port=5000\n";
        newFile << "dictionary=ru_words.txt\n";
        newFile.close();
        // возвращаем значения по умолчанию
        config["ip"] = "127.0.0.1";
        config["port"] = "5000";
        config["dictionary"] = "ru_words.txt";
        return config;
    }

    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;
        size_t pos = line.find('=');
        if (pos == string::npos) continue;
        string key = line.substr(0, pos);
        string value = line.substr(pos + 1);
        config[key] = value;
    }
    return config;
}
