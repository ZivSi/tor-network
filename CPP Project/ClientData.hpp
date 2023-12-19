#pragma once

#include <string>
#include <secblock.h>

#include <WS2tcpip.h>
#include <WinSock2.h>

#include "nlohmann/json.hpp"
#include "AesHandler.h"

using std::string;
using CryptoPP::SecByteBlock;

class ClientData {
public:
	ClientData(unsigned long long last_seen, unsigned long long int bytes_sent, unsigned long long int bytes_received, SecByteBlock aes_key, SecByteBlock aes_iv) {
		this->last_seen = last_seen;
		this->bytes_sent = bytes_sent;
		this->bytes_received = bytes_received;
		this->aes_key = aes_key;
		this->aes_iv = aes_iv;
	}

	ClientData(unsigned long long last_seen, unsigned long long int bytes_sent, unsigned long long int bytes_received, SecByteBlock aes_key, SecByteBlock aes_iv, SOCKET socket) {
		this->last_seen = last_seen;
		this->bytes_sent = bytes_sent;
		this->bytes_received = bytes_received;
		this->aes_key = aes_key;
		this->aes_iv = aes_iv;
		this->socket = socket;
	}

	ClientData() {
		this->last_seen = 0;
		this->bytes_sent = 0;
		this->bytes_received = 0;
	}

	unsigned long long get_last_seen() {
		return this->last_seen;
	}

	unsigned long long int get_bytes_sent() {
		return this->bytes_sent;
	}

	unsigned long long int get_bytes_received() {
		return this->bytes_received;
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

	void set_last_seen(unsigned long long last_seen) {
		this->last_seen = last_seen;
	}

	void set_bytes_sent(unsigned long long int bytes_sent) {
		this->bytes_sent = bytes_sent;
	}

	void set_bytes_received(unsigned long long int bytes_received) {
		this->bytes_received = bytes_received;
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

	string toString() const {
		nlohmann::json jsonObj;

		jsonObj["last_seen"] = last_seen;
		jsonObj["bytes_sent"] = bytes_sent;
		jsonObj["bytes_received"] = bytes_received;
		jsonObj["aes_key"] = AesHandler::SecByteBlockToString(aes_key);
		jsonObj["aes_iv"] = AesHandler::SecByteBlockToString(aes_iv);
		jsonObj["socket"] = socket;

		return jsonObj.dump();
	}

	static ClientData fromString(const string& jsonString) {
		nlohmann::json jsonObj = nlohmann::json::parse(jsonString);

		ClientData newData;

		newData.last_seen = jsonObj["last_seen"].get<unsigned long long>();
		newData.bytes_sent = jsonObj["bytes_sent"].get<unsigned long long>();
		newData.bytes_received = jsonObj["bytes_received"].get<unsigned long long>();

		// Assuming SecByteBlock constructor takes a byte pointer and size
		newData.aes_key = AesHandler::StringToSecByteBlock(jsonObj["aes_key"].get<string>());
		newData.aes_iv = AesHandler::StringToSecByteBlock(jsonObj["aes_iv"].get<string>());

		newData.socket = jsonObj["socket"].get<SOCKET>();

		return newData;
	}

	bool operator==(const ClientData& other) const {
		return this->socket == other.socket;
	}

private:
	unsigned long long last_seen;
	unsigned long long int bytes_sent;
	unsigned long long int bytes_received;

	// AES
	SecByteBlock aes_key;
	SecByteBlock aes_iv;

	SOCKET socket;
};
