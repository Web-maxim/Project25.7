// User.cpp
#include "User.h"

User::User(const string& _name, const string& _login, const uint* _pass)
	: name(_name), login(_login) {
	// Копирование в std::array вместо выделения памяти
	for (int i = 0; i < 5; ++i) pass_hash[i] = _pass[i];
}

bool User::prov(const string& pass) {
	uint* digest = sha1(pass, static_cast<uint>(pass.size()));
	bool isValid = true;
	for (int i = 0; i < 5; i++) {
		if (digest[i] != pass_hash[i]) { isValid = false; break; }
	}
	delete[] digest;
	return isValid;
}

const array<uint, 5>& User::get_hash() const {  
	return pass_hash;
}