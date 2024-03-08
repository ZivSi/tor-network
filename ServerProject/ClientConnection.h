#pragma once

#include "AesHandler.h"
#include "ECCHandler.h"
#include "Logger.h"
#include "Utility.h"
#include <errno.h> // for errno
#include <exception>
#include <fcntl.h>
#include <string>
#include <sys/types.h>
#include <vector>

#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

using std::string;
using std::to_string;
using std::exception;

class ClientConnection
{
public:
	ClientConnection(string ip, unsigned short port);
	ClientConnection(string ip, unsigned short port, Logger logger);
	ClientConnection(string ip, unsigned short port, Logger logger, ECCHandler* eccHandler);
	~ClientConnection();

	void initWSASocket();
	SOCKET connectToServer();
	SOCKET connectInLoop();

	void handshake();

	void sendData(string data);
	void sendKeys(string keysStr);

	void sendEncrypted(string data);

	string receiveDataFromTcp(); // Receive data without getting the size first
	string receiveData();
	string receiveKeys(bool initialize);

	void initializeParentECC(string receivedECCKeys);

	void sendECCKeys();
	void sendECCKeys(ECCHandler* eccHandler); // If already has one of its own
	void sendAESKeys();

	SOCKET getSocket() const;
	unsigned short getPort() const;
	string getIP() const;

	AesKey getAesKey() const;
	ECCHandler* getParentECCHandler();

	AesHandler* getAesHandler();

	void closeConnection();

	bool isConnectionActive() const;

private:
	const int MAX_ATTEMPTS = 3;
	const int DELAY_IN_CONNECTION_LOOP = 1000;

	int failedAttempts = 0;

	SOCKET connection;
	string ip;
	unsigned short port;

	Logger logger;

	ECCHandler parentECCHandler; // Parent key
	ECCHandler eccHandler; // My key

	AesHandler aesHandler;

	bool connectionActive = false;
};

