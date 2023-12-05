#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

using std::string;

class ClientData {
public:
	ClientData(string username, string password, string email, long long int bytes_sent, long long int bytes_received, string aes_key, string aes_iv) {
		this->username = username;
		this->password = password;
		this->email = email;
		this->bytes_sent = bytes_sent;
		this->bytes_received = bytes_received;
		this->aes_key = aes_key;
		this->aes_iv = aes_iv;
	}

	ClientData() {
		this->username = "";
		this->password = "";
		this->email = "";
		this->bytes_sent = 0;
		this->bytes_received = 0;
		this->aes_key = "";
		this->aes_iv = "";
	}

	string get_username() {
		return this->username;
	}

	string get_password() {
		return this->password;
	}

	string get_email() {
		return this->email;
	}

	long long int get_bytes_sent() {
		return this->bytes_sent;
	}

	long long int get_bytes_received() {
		return this->bytes_received;
	}

	string get_aes_key() {
		return this->aes_key;
	}

	string get_aes_iv() {
		return this->aes_iv;
	}


	void set_username(string username) {
		this->username = username;
	}

	void set_password(string password) {
		this->password = password;
	}

	void set_email(string email) {
		this->email = email;
	}

	void set_bytes_sent(long long int bytes_sent) {
		this->bytes_sent = bytes_sent;
	}

	void set_bytes_received(long long int bytes_received) {
		this->bytes_received = bytes_received;
	}

	void set_aes_key(string aes_key) {
		this->aes_key = aes_key;
	}

	void set_aes_iv(string aes_iv) {
		this->aes_iv = aes_iv;
	}

private:
	string username;
	string password;
	string email;
	long long int bytes_sent;
	long long int bytes_received;

	// AES
	string aes_key;
	string aes_iv;

};

#endif