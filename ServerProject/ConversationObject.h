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
	unsigned long long creationTime;

	SOCKET prvNodeSocket; // Previous node is socket because we are it's server
	unsigned short prvPort;
	// TODO: add ip

	ClientConnection* nxtNode;
	unsigned short nxtPort;
	// TODO: add ip

	string conversationId; // UUID
	AesKey key; // Agreed between client and current node

	static string LETTERS;

	bool exitNode = false;

public:
	ConversationObject();
	ConversationObject(SOCKET prvNode, ClientConnection* nxtNode, string conversationId, AesKey key);
	ConversationObject(string conversationId, AesKey key);
	~ConversationObject();

	SOCKET getPrvNodeSOCKET() const;
	ClientConnection* getNxtNode();

	unsigned short getPrvPort();
	// TODO: add ip getter

	unsigned short getNxtPort() const;
	// TODO: add ip getter

	string getConversationId();
	AesKey* getKey();

	void setPrvNode(SOCKET prvNodeSocket);
	void setNxtNode(ClientConnection* nxtNode);

	void setPrvPort(unsigned short prvPort);
	// TODO: add ip setter
	void setNxtPort(unsigned short nxtPort);
	// TODO: add ip setter

	void setConversationId(string conversationId);
	void setKey(AesKey key);

	void setAsExitNode();

	static string generateID();

	bool isEmpty() const;

	bool isExitNode() const;

	bool isTooOld() const;
};