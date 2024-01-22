#pragma once

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <string>
#include "Logger.h"
#include <string>

using std::cerr;
using std::string;
using std::to_string;

enum class KeyType
{
	AES,
	ECC
};

class IConnection
{
public:
	IConnection(string ip, unsigned short port, Logger logger);
	virtual ~IConnection();

	SOCKET initWSASocket();
	void bindSocket(SOCKET socket);
	void listenSocket(SOCKET socket);

	virtual void acceptSocket(SOCKET socket) = 0;
	virtual void handleConnection(SOCKET clientSocket) = 0;

	void sendData(SOCKET connection, string data);
	void sendKeys(SOCKET connection, string keyStr);

	string receiveData(SOCKET connection);
	string receiveKeys(SOCKET connection);

	void closeConnection();

protected:
	SOCKET getSocket();
	unsigned short getPort();
	string getIP();


private:
	SOCKET connection;
	unsigned short port;
	string ip;

	Logger logger;
};

