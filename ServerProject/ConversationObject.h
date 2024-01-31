#pragma once

#include "AesHandler.h"
#include <string>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <random>

#define UUID_LEN 4 // Good enough

using std::string;

class ConversationObject
{
private:
	SOCKET prvNode;
	unsigned short prvPort;

	SOCKET nxtNode;
	unsigned short nxtPort;

	string conversationId; // UUID
	AesKey key; // Agreed between client and current node

	static string LETTERS;

public:
	ConversationObject();
	ConversationObject(SOCKET prvNode, SOCKET nxtNode, string conversationId, AesKey key);
	ConversationObject(string conversationId, AesKey key);
	~ConversationObject();

	SOCKET getPrvNode();
	SOCKET getNxtNode();

	unsigned short getPrvPort();
	unsigned short getNxtPort();

	string getConversationId();
	AesKey getKey();

	void setPrvNode(SOCKET prvNode);
	void setNxtNode(SOCKET nxtNode);

	void setPrvPort(unsigned short prvPort);
	void setNxtPort(unsigned short nxtPort);

	void setConversationId(string conversationId);
	void setKey(AesKey key);

	static string generateID();

	bool isEmpty();
};