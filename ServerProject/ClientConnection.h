#pragma once

#include <string>
#include "Logger.h"
#include <exception>


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
	SOCKET connectToServer(string ip, unsigned short port);
	SOCKET connectInLoop(string ip, unsigned short port);

	void sendData(string data);
	void sendKeys(string keysStr);

	string receiveData();
	string receiveKeys();

	SOCKET getSocket();
	unsigned short getPort();
	string getIP();

	void closeConnection();

private:
	SOCKET connection;
	string ip;
	unsigned short port;

	Logger logger;
};

