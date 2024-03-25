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
	// ----------------- Constructors & Destructor -----------------
	ClientConnection(string ip, unsigned short port);
	ClientConnection(string ip, unsigned short port, Logger logger);
	ClientConnection(string ip, unsigned short port, Logger logger, ECCHandler* eccHandler);
	~ClientConnection();

	// ----------------- Initialization -----------------
	void initWSASocket();

	// ----------------- Connection Handling -----------------
	SOCKET connectToServer();
	SOCKET connectInLoop();
	void handshake();
	void closeConnection();

	// ----------------- Data Transmission -----------------
	void sendData(string data);
	void sendKeys(string keysStr);
	void sendEncrypted(string data);
	void sendDataTcp(string data); // Send without sending the size first
	string receiveDataFromTcp(bool block = false); // Receive data without getting the size first
	string receiveData();
	string receiveKeys(bool initialize);

	// ----------------- Key Exchange -----------------
	void initializeParentECC(string receivedECCKeys);
	void sendECCKeys();
	void sendECCKeys(ECCHandler* eccHandler); // If already has one of its own
	void sendAESKeys();

	// ----------------- Getters -----------------
	SOCKET getSocket() const;
	unsigned short getPort() const;
	string getIP() const;
	AesKey getAesKey() const;
	ECCHandler* getParentECCHandler();
	AesHandler* getAesHandler();

	// ----------------- Utility -----------------
	bool isConnectionActive() const;

private:
	// ----------------- Constants -----------------
	const int MAX_ATTEMPTS = 10;
	const int DELAY_IN_CONNECTION_LOOP = 1000;

	// ----------------- Variables -----------------
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
