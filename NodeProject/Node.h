#pragma once

#include "AesHandler.h"
#include "ClientConnection.h"
#include "Constants.h"
#include "ConversationObject.h"
#include "ECCHandler.h"
#include "IConnection.h"
#include "Logger.h"
#include "Utility.h"

#include "unordered_map"
#include <chrono>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include <xstring>

#include <cstring> // For memset
// Encryptor
#include <cryptlib.h>

#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

using std::string;
using std::vector;
using std::unordered_map;
using std::cout;
using std::cerr;
using std::endl;
using std::thread;
using std::to_string;

using CryptoPP::ECP;
using CryptoPP::ECIES;

using namespace Constants;


class Node : public IConnection
{
public:
	Node();
	Node(string parentIP, unsigned short parentPort);
	~Node();

	/*
	* Run server with increase of the PORT (Choose this to start the server)
	*/
	void start();

private:
	Logger logger;

	/*
	* List of active conversations the server involved in
	*/
	unordered_map<string, ConversationObject*> conversationsMap; // Conversations involved in

	/*
	* Static variable to set the ports to each node
	*/
	static unsigned short PORT;

	/*
	* The node's server socket
	*/
	bool stop;
	string myIP;
	unsigned short myPort;

	/*
	* The socket of the parent server
	*/
	ClientConnection* parentConnection;

	/*
	* Set actions to perform when new connection is made
	*/
	void acceptSocket(SOCKET socket) override;

	/*
	* Handle node or client connected
	* If client - receive AES, generate conversation ID and perform handshake
	* If node - extract current conversation id, decrypt, send
	*/
	void handleClient(SOCKET clientSocket) override;

	bool conversationExists(ConversationObject* currentConversation);


	/*
	* The conversation will start with ECC key
	*/
	bool isHandshake(string received);
	string buildAliveFormat();

	void clientHandshake(SOCKET clientSocket);

	/*
	* With parent. Send formatted alive message
	*/
	void handshake(ClientConnection* parentConnection);
	void sendAESKeys(ClientConnection* parentConnection, string receivedECCKeys);

	void listenToNextNode(SOCKET nodeSocket, ConversationObject* currentConversation); // For reverse conversation
	void listenToHosts(ConversationObject* currentConversation);
	void handleNode(SOCKET nodeSocket, ConversationObject* currentConversation, string initialMessage);
	void handleNodeAsExit(SOCKET nodeSocket, ConversationObject* currentConversation, string initialMessage);

	void collectAndSendReversedMessages(ConversationObject* currentConversation, const SOCKET& nodeSocket);

	void sendAlive();

	string receiveECCKeys(SOCKET clientSocket);

	ConversationObject* findConversationBy(string conversationId);
	bool isConnectedTo(ClientConnection* nextNode);

	void removeConversationFromMap(string conversationId);

	string getLocalIpv4();

	bool dataLegit(string& data);
};