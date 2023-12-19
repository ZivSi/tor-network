#pragma once

#include <string>
#include <secblock.h>

#include <WS2tcpip.h>
#include <WinSock2.h>

using std::string;
using CryptoPP::SecByteBlock;


// This class will store data only on runtime - meaning each node that connected to the server will have an instance of this class - but will not be stored in the database
class NodeData {
public:
	NodeData(string server_port, unsigned long long last_seen, string public_key, SecByteBlock aes_key, SecByteBlock aes_iv) {
		this->server_port = server_port;
		this->last_seen = last_seen;
		this->public_key = public_key;
		this->aes_key = aes_key;
		this->aes_iv = aes_iv;
	}

	NodeData(string server_port, unsigned long long last_seen, string public_key, SecByteBlock aes_key, SecByteBlock aes_iv, SOCKET socket) {
		this->server_port = server_port;
		this->last_seen = last_seen;
		this->public_key = public_key;
		this->aes_key = aes_key;
		this->aes_iv = aes_iv;
		this->socket = socket;
	}

	NodeData() {
	}

	string get_server_port() {
		return this->server_port;
	}

	unsigned long long get_last_seen() {
		return this->last_seen;
	}

	string get_public_key() {
		return this->public_key;
	}

	SecByteBlock get_aes_key() {
		return this->aes_key;
	}

	SecByteBlock get_aes_iv() {
		return this->aes_iv;
	}

	SOCKET get_socket() {
		return this->socket;
	}

	unsigned long long get_sent_bytes() {
		return this->sent_bytes;
	}

	unsigned long long get_received_bytes() {
		return this->received_bytes;
	}

	void set_server_port(string server_port) {
		this->server_port = server_port;
	}

	void set_last_seen(unsigned long long last_seen) {
		this->last_seen = last_seen;
	}

	void set_public_key(string public_key) {
		this->public_key = public_key;
	}

	void set_aes_key(SecByteBlock aes_key) {
		this->aes_key = aes_key;
	}

	void set_aes_iv(SecByteBlock aes_iv) {
		this->aes_iv = aes_iv;
	}

	void set_socket(SOCKET socket) {
		this->socket = socket;
	}

	void set_sent_bytes(unsigned long long sent_bytes) {
		this->sent_bytes = sent_bytes;
	}

	void set_received_bytes(unsigned long long received_bytes) {
		this->received_bytes = received_bytes;
	}

	// Operator == overloading
	bool operator==(const NodeData& node_data) const {
		return (this->server_port == node_data.server_port && this->last_seen == node_data.last_seen && this->public_key == node_data.public_key && this->aes_key == node_data.aes_key && this->aes_iv == node_data.aes_iv && this->socket == node_data.socket);
	}

private:
	string server_port;
	unsigned long long last_seen;
	string public_key;

	// AES
	SecByteBlock aes_key;
	SecByteBlock aes_iv;

	SOCKET socket;

	unsigned long long sent_bytes = 0;
	unsigned long long received_bytes = 0;
};
