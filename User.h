// User.h
#pragma once
// Безопасное хранение хеша — std::array 
#include "sha1.h"
#include <string>
#include <array>
#include <cstring> //можно удалить, так как нет memcpy
using namespace std;

class User {
public:
	const array<uint, 5>& get_hash() const;  // добавлено
	User(const string& _name, const string& _login, const uint* pass);
	~User() = default;

	bool prov(const string& pass);

	string name;


private:
	string login;
	array<uint, 5> pass_hash{}; // Изменено: было uint* с new[]/delete[]
};