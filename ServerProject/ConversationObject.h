#pragma once

#include "AesHandler.h"
#include <string>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <random>
#include "ClientConnection.h"

#define UUID_LEN 4 // Good enough

using std::string;

/*
A class to be stored on the nodes. Each node can have multiple conversations it's involved in.
*/

class ConversationObject
{
private:
	ClientConnection* prvNode;
	unsigned short prvPort;

	ClientConnection* nxtNode;
	unsigned short nxtPort;

	string conversationId; // UUID
	AesKey key; // Agreed between client and current node

	static string LETTERS;

public:
	ConversationObject();
	ConversationObject(ClientConnection* prvNode, ClientConnection* nxtNode, string conversationId, AesKey key);
	ConversationObject(string conversationId, AesKey key);
	~ConversationObject();

	ClientConnection* getPrvNode();
	ClientConnection* getNxtNode();

	unsigned short getPrvPort();
	unsigned short getNxtPort();

	string getConversationId();
	AesKey* getKey();

	void setPrvNode(ClientConnection* prvNode);
	void setNxtNode(ClientConnection* nxtNode);

	void setPrvPort(unsigned short prvPort);
	void setNxtPort(unsigned short nxtPort);

	void setConversationId(string conversationId);
	void setKey(AesKey key);

	static string generateID();

	bool isEmpty();
};