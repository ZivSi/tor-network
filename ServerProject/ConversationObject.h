#pragma once

#include "AesHandler.h"
#include "ClientConnection.h"
#include <map>
#include <queue>
#include <random>
#include <string>
#include <WinSock2.h>
#include <WS2tcpip.h>

#define UUID_LEN 4 // Good enough

using std::string;
using std::queue;
using std::map;

class DestinationData
{
private:
	string destinationIP;
	unsigned short destinationPort;
	string data;

public:
	const int IP_INDEX = 0;
	const int PORT_INDEX = 1;
	const int DATA_INDEX = 2;

	DestinationData(string destinationIP, unsigned short destinationPort, string data);
	DestinationData(string decryptedData);
	~DestinationData();

	string getDestinationIP();
	unsigned short getDestinationPort();
	string getData();

	ClientConnection* createConnection();
};


// Will be used to represent the active connection the current client is in
class ConnectionPair
{
private:
	string ip;
	unsigned short port;

public:
	ConnectionPair(string ip, unsigned short port);
	~ConnectionPair();

	string getIP() const;
	unsigned short getPort() const;

	bool operator==(const ConnectionPair& other) const;
	bool operator<(const ConnectionPair& other) const;
};

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

	queue<string> messageQueue; // Encrypted messages

	map<ConnectionPair, ClientConnection*> destinationMap;

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

	void addMessage(string message);
	bool isQueueEmpty() const;
	string getFirstMessage();

	void setPrvNodeSOCKET(SOCKET prvNodeSocket);
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

	// Iterate through the connection, check if there are messages to collect. If so, collect them, encrypt and add to the queue to handle later
	void collectMessages();

	ClientConnection* addActiveConnection(DestinationData destinationData);
	void removeActiveConnection(DestinationData destinationData);
	ClientConnection* getActiveConnection(DestinationData destinationData);
	bool isDestinationActive(DestinationData destinationData);
};