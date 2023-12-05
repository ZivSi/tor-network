#pragma once

#include <string>

using std::string;


// This class will store data only on runtime - meaning each node that connected to the server will have an instance of this class - but will not be stored in the database
class NodeData {
public:
	NodeData(string server_port, string last_seen, string public_key, string aes_key, string aes_iv) {
		this->server_port = server_port;
		this->last_seen = last_seen;
		this->public_key = public_key;
		this->aes_key = aes_key;
		this->aes_iv = aes_iv;
	}

	string get_server_port() {
		return this->server_port;
	}

	string get_last_seen() {
		return this->last_seen;
	}

	string get_public_key() {
		return this->public_key;
	}

	string get_aes_key() {
		return this->aes_key;
	}

	string get_aes_iv() {
		return this->aes_iv;
	}

	void set_server_port(string server_port) {
		this->server_port = server_port;
	}

	void set_last_seen(string last_seen) {
		this->last_seen = last_seen;
	}

	void set_public_key(string public_key) {
		this->public_key = public_key;
	}

	void set_aes_key(string aes_key) {
		this->aes_key = aes_key;
	}

	void set_aes_iv(string aes_iv) {
		this->aes_iv = aes_iv;
	}

private:
	string server_port;
	string last_seen;
	string public_key;
	string aes_key;
	string aes_iv;
};
