#pragma once

#include "AesHandler.h"
#include <string>
#include <iostream>
#include "ECCHandler.h"

using std::string;
using std::cout;
using std::endl;


class RelayObject
{
private:
	unsigned short port;
	AesKey aesKey;
	ECCHandler* eccHandler;
	string conversationId;
	string conversationIdEncrypted;

public:
	RelayObject(unsigned short port, AesKey aesKeys, ECCHandler* eccHandler, string conversationId);
	RelayObject(unsigned short port);
	~RelayObject();

	unsigned short getPort();
	AesKey* getAesKeys();
	ECCHandler* getEccHandler();
	string getConversationId();
	string getConversationIdEncrypted();

	void setConversationId(string conversationId);
};

