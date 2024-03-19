#pragma once

#include "../ServerProject/AesHandler.h"
#include "../ServerProject/ECCHandler.h"
#include <iostream>
#include <string>

using std::string;
using std::cout;
using std::endl;


class RelayObject
{
private:
	string ip;
	unsigned short port;
	AesKey aesKey;
	ECCHandler* eccHandler;
	string conversationId;
	string conversationIdEncrypted;

public:
	RelayObject(string ip, unsigned short port, AesKey aesKeys, ECCHandler* eccHandler, string conversationId);
	RelayObject(string ip, unsigned short port);  // TODO: Change
	~RelayObject();

	string getIp();
	unsigned short getPort();
	AesKey* getAesKeys();
	ECCHandler* getEccHandler();
	string getConversationId();
	string getConversationIdEncrypted();

	void setAesKeys(AesKey aesKeys);
	void setEccHandler(ECCHandler* eccHandler);
	void setConversationId(string conversationId);

	string toString();
};


class RelayProperties
{
private:
	string ip;
	unsigned short port;

public:
	RelayProperties(string ip, unsigned short port);
	RelayProperties();
	~RelayProperties();

	string getIp();
	unsigned short getPort();
	void setIp(string ip);
	void setPort(unsigned short port);
};