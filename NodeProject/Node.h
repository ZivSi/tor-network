#pragma once

#include "../ServerProject/AesHandler.h"
#include "../ServerProject/ClientConnection.h"
#include "../ServerProject/Constants.h"
#include "../ServerProject/ConversationObject.h"
#include "../ServerProject/ECCHandler.h"
#include "../ServerProject/IConnection.h"
#include "../ServerProject/Logger.h"
#include "../ServerProject/Utility.h"
#include "../ServerProject/RSAHandler.h"
#include "JsonResponse.h"

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
	// ----------------- Constructors & Destructor -----------------
	Node();
	Node(string parentIP, unsigned short parentPort);
	~Node();

	// ----------------- Main Method -----------------
	void start();

private:
	Logger logger;
	RSAHandler rsaHandler;

	// ----------------- Variables -----------------
	unordered_map<string, ConversationObject*> conversationsMap; // Conversations involved in
	static unsigned short PORT;
	bool stop;
	string myIP;
	unsigned short myPort;
	ClientConnection* parentConnection;

	// ----------------- Connection Handling -----------------
	void acceptSocket(SOCKET socket) override;
	void handleClient(SOCKET clientSocket) override;

	void listenToNextNode(SOCKET nodeSocket, ConversationObject* currentConversation);
	void listenToHosts(ConversationObject* currentConversation);
	void handleNode(SOCKET previousNodeSocket, ConversationObject* currentConversation, string initialMessage);
	void handleNodeAsExit(SOCKET previousNodeSocket, ConversationObject* currentConversation, string initialMessage);

	// ----------------- Handshake Related -----------------
	bool isHandshake(string received);
	void clientHandshake(SOCKET clientSocket);
	void handshake(ClientConnection* parentConnection);
	void sendAESKeys(ClientConnection* parentConnection, string receivedECCKeys);
	string receiveECCKeys(SOCKET clientSocket);


	void sendMessageToClient(ConversationObject* conversation, string hostIp, unsigned short hostPort, int messageCode, string message);
	// ----------------- Error Handling -----------------
	void sendCouldNotConnectToHost(ConversationObject* conversation, string hostIp, unsigned short hostPort);
	void sendConversationTimeout(ConversationObject* conversation);
	void sendNodeUnreachable(ConversationObject* conversation);
	// ----------------- Utility -----------------
	string buildAliveFormat();
	bool dataLegit(string& data);

	// ----------------- Getters -----------------
	string getLocalIpv4();

	// ----------------- Helper Methods -----------------
	bool conversationExists(ConversationObject* currentConversation);
	ConversationObject* findConversationBy(string conversationId);
	bool isConnectedTo(ClientConnection* nextNode);
	void removeConversationFromMap(string conversationId);

	// ----------------- Connection Alive -----------------
	void sendAlive();

	// ----------------- Mapping Server -----------------
	string getPropertiesByUsername(string username);
};
