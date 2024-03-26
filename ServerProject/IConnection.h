#pragma once

#include "ECCHandler.h"
#include "Logger.h"
#include <iostream>
#include <mutex>
#include <string>
#include <string>
#include <WinSock2.h>
#include <WS2tcpip.h>

using std::cerr;
using std::string;
using std::to_string;
using std::mutex;

enum class KeyType
{
	AES,
	ECC
};

class IConnection
{
public:
	IConnection(string ip, unsigned short port, Logger* logger);
	virtual ~IConnection();

	SOCKET initWSASocket();
	void bindSocket(SOCKET socket);
	void listenSocket(SOCKET socket);

	virtual void acceptSocket(SOCKET socket) = 0;
	virtual void handleClient(SOCKET clientSocket) = 0;

	void sendData(SOCKET connection, const string& data);
	void sendKeys(SOCKET connection, const string& keyStr);

	void sendECCKey(SOCKET connection);

	string receiveData(SOCKET connection);
	string receiveKeys(SOCKET connection);

	void closeConnection();

	string getLocalIpv4();

	SOCKET getSocket() const;
	unsigned short getPort() const;
	string getIP() const;
	ECCHandler* getECCHandler();

protected:
	string encryptECC(string data);
	string decryptECC(string data);

private:
	SOCKET connection;
	unsigned short port;
	string ip;

	Logger* logger;

	mutex eccHandlerMutex;
	ECCHandler eccHandler;
};

