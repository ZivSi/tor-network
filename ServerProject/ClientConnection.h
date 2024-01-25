#pragma once

#include <string>
#include "Logger.h"
#include <exception>
#include "ECCHandler.h"
#include "AesHandler.h"

#include <WS2tcpip.h>
#include <WinSock2.h>

#pragma comment(lib, "Ws2_32.lib")

using std::string;
using std::to_string;
using std::exception;

class ClientConnection
{
public:
	ClientConnection(string ip, unsigned short port, Logger logger);
	~ClientConnection();

	void initWSASocket();
	SOCKET connectToServer();
	SOCKET connectInLoop();

	void handshake();

	void sendData(string data);
	void sendKeys(string keysStr);

	void sendEncrypted(string data);

	string receiveData();
	string receiveKeys(bool initialize);

	void initializeParentECC(string receivedECCKeys);

	void sendECCKeys();
	void sendAESKeys();

	SOCKET getSocket();
	unsigned short getPort();
	string getIP();

	void closeConnection();

private:
	SOCKET connection;
	string ip;
	unsigned short port;

	Logger logger;

	ECCHandler parentECCHandler; // Parent key
	ECCHandler eccHandler; // My key

	AesHandler aesHandler;
};

