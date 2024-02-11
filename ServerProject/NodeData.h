#pragma once

#include "Constants.h"
#include "ECCHandler.h"
#include "Utility.h"
#include <cryptlib.h>
#include <eccrypto.h>
#include <hex.h>
#include <integer.h>
#include <string>
#include <vector>

using std::string;
using std::to_string;
using std::vector;
using CryptoPP::ECDSA;
using CryptoPP::ECP;
using CryptoPP::SHA256;
using CryptoPP::ECIES;

/*
Class that will be stored on the server.
*/

class NodeData {
private:
	unsigned short port;
	// TODO: add ip
	ECCHandler ecchandler;

	unsigned long long lastAliveMessageTime;
	unsigned long aliveMessagesCount;
	unsigned long averageResponseTime;

public:
	NodeData(unsigned short port); // TODO: add ip
	NodeData(unsigned short port, string serializedPublicKey); // TODO: add ip
	NodeData(unsigned short port, string serializedPublicKey, unsigned long long lastAliveMessageTime, unsigned long aliveMessagesCount, unsigned long averageResponseTime); // TODO: add ip
	NodeData();
	~NodeData();

	unsigned short getPort();
	// TODO: add ip getter
	ECCHandler* getECCHandler();
	unsigned long long getLastAliveMessageTime();
	unsigned long getAliveMessagesCount();
	unsigned long getAverageResponseTime();

	void setPort(unsigned short port);
	// TODO: add ip setter
	void setECCKey(string serializedPublicKey);
	void setLastAliveMessageTime(unsigned long long lastAliveMessageTime);
	void updateLastAliveMessageTime();
	void setAliveMessagesCount(unsigned long aliveMessagesCount);
	void setAverageResponseTime(unsigned long averageResponseTime);

	string encrypt(string plaintext);

	string toString();
	vector<unsigned char> toSend();

	bool isEmpty();

	operator int() const;
	bool operator==(const NodeData& other) const;
};
