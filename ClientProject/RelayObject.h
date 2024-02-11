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
	// TODO: Change. add ip
	unsigned short port;
	AesKey aesKey;
	ECCHandler* eccHandler;
	string conversationId;
	string conversationIdEncrypted;

public:
	RelayObject(unsigned short port, AesKey aesKeys, ECCHandler* eccHandler, string conversationId);  // TODO: Change
	RelayObject(unsigned short port);  // TODO: Change
	~RelayObject();

	unsigned short getPort();
	// TODO: Change. add getIp
	AesKey* getAesKeys();
	ECCHandler* getEccHandler();
	string getConversationId();
	string getConversationIdEncrypted();

	void setAesKeys(AesKey aesKeys);
	void setEccHandler(ECCHandler* eccHandler);
	void setConversationId(string conversationId);
};

