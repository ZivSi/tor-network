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

// Class that represents data in the format "IP:PORT:DATA" for easy access
class DestinationData
{
private:
	// ----------------- Member Variables -----------------
	string destinationIP;
	unsigned short destinationPort;
	string data;

	// ----------------- Constants -----------------
	static const int IP_INDEX = 0;
	static const int PORT_INDEX = 1;
	static const int DATA_INDEX = 2;

public:
	// ----------------- Constructors & Destructor -----------------
	DestinationData(string destinationIP, unsigned short destinationPort, string data);
	DestinationData(string decryptedData);
	DestinationData(vector<string>* splitData);
	~DestinationData();

	// ----------------- Getters -----------------
	string getDestinationIP();
	unsigned short getDestinationPort();
	string getData();

	// ----------------- Connection Creation -----------------
	ClientConnection* createConnection();
};

// Class to represent an active connection
class ConnectionPair
{
private:
	// ----------------- Member Variables -----------------
	string ip;
	unsigned short port;

public:
	// ----------------- Constructors & Destructor -----------------
	ConnectionPair(string ip, unsigned short port);
	ConnectionPair(string properties);
	ConnectionPair();
	~ConnectionPair();

	// ----------------- Getters -----------------
	string getIP() const;
	unsigned short getPort() const;

	// ----------------- Operator Overloading -----------------
	bool operator==(const ConnectionPair& other) const;
	bool operator<(const ConnectionPair& other) const;

	friend std::ostream& operator<<(std::ostream& os, const ConnectionPair& obj);

	string toString() const;
};

// Object to hold data of a client's conversation
class ConversationObject
{
private:
	// ----------------- Member Variables -----------------
	unsigned long long creationTime;
	SOCKET prvNodeSocket; // Previous node socket
	string prvIP;
	unsigned short prvPort;
	ClientConnection* nxtNode;
	string nxtIP;
	unsigned short nxtPort;
	string conversationId; // UUID
	AesKey key;
	static string LETTERS;
	bool exitNode = false;
	queue<string> messageQueue;
	map<ConnectionPair, ClientConnection*> destinationMap;

public:
	// ----------------- Constructors & Destructor -----------------
	ConversationObject();
	ConversationObject(SOCKET prvNode, ClientConnection* nxtNode, string conversationId, AesKey key);
	ConversationObject(string conversationId, AesKey key);
	~ConversationObject();

	// ----------------- Getters -----------------
	SOCKET getPrvNodeSOCKET() const;
	ClientConnection* getNxtNode();
	string getPrvIP() const;
	unsigned short getPrvPort();
	string getNxtIP() const;
	unsigned short getNxtPort() const;
	string getConversationId();
	AesKey* getKey();

	// ----------------- Setters -----------------
	void setPrvNodeSOCKET(SOCKET prvNodeSocket);
	void setNxtNode(ClientConnection* nxtNode);
	void setPrvIP(string prvIP);
	void setPrvPort(unsigned short prvPort);
	void setNxtIP(string nxtIP);
	void setNxtPort(unsigned short nxtPort);
	void setConversationId(string conversationId);
	void setKey(AesKey key);
	void setAsExitNode();

	// ----------------- Utility -----------------
	static string generateID();
	bool isEmpty() const;
	bool isExitNode() const;
	bool isTooOld() const;
	void collectMessages();
	ClientConnection* addActiveConnection(DestinationData destinationData);
	void removeActiveConnection(DestinationData destinationData);
	ClientConnection* getActiveConnection(DestinationData destinationData);
	bool isDestinationActive(DestinationData destinationData);


	// ----------------- Queue Related -----------------
	bool isQueueEmpty() const;
	size_t getQueueSize() const;
	void addMessage(string message);
	string getFirstMessage();
};
