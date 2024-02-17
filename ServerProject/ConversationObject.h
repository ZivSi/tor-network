#pragma once

#include "AesHandler.h"
#include "ClientConnection.h"
#include <random>
#include <string>
#include <WinSock2.h>
#include <WS2tcpip.h>

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
	string prvIP;
	unsigned short prvPort;

	ClientConnection* nxtNode;
	string nxtIP;
	unsigned short nxtPort;

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

	string getPrvIP() const;
	unsigned short getPrvPort();

	string getNxtIP() const;
	unsigned short getNxtPort() const;

	string getConversationId();
	AesKey* getKey();

	void setPrvNode(SOCKET prvNodeSocket);
	void setNxtNode(ClientConnection* nxtNode);

	void setPrvIP(string prvIP);
	void setPrvPort(unsigned short prvPort);

	void setNxtIP(string nxtIP);
	void setNxtPort(unsigned short nxtPort);

	void setConversationId(string conversationId);
	void setKey(AesKey key);

	void setAsExitNode();

	static string generateID();

	bool isEmpty() const;

	bool isExitNode() const;

	bool isTooOld() const;
};