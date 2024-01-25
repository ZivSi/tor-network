#pragma once

#include "AesHandler.h"
#include "ConversationObject.h"
#include "ECCHandler.h"
#include "Constants.h"
#include "Utility.h"
#include "Logger.h"
#include "ClientConnection.h"
#include "IConnection.h"

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <xstring>

// Encryptor
#include <cryptlib.h>

#include <WS2tcpip.h>
#include <WinSock2.h>

#pragma comment(lib, "Ws2_32.lib")

using std::string;
using std::vector;
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
	~Node();

	/*
	* Run server with increase of the PORT (Choose this to start the server)
	*/
	void start();

private:
	/*
	* List of active conversations the server involved in
	*/
	vector<ConversationObject> conversations;
	ECCHandler eccHandler;
	AesHandler aesHandler;

	Logger logger;

	/*
	* Static variable to set the ports to each node
	*/
	static unsigned short PORT;

	/*
	* The node's server socket
	*/
	SOCKET myServerSocket;
	bool stop;
	unsigned short myPort;

	/*
	* The socket of the parent server
	*/
	ClientConnection* parentConnection;

	/*
	* Set actions to perform when new connection is made
	*/
	void acceptSocket(SOCKET socket) override;
	ClientConnection* connectToParent(string parentIp, unsigned short parentPort, bool repeat);

	/*
	* Handle node or client connected
	* If client - receive AES, generate conversation ID and perform handshake
	* If node - extract current conversation id, decrypt, send
	*/
	void handleClient(SOCKET clientSocket) override;


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

	void sendAlive();
};

