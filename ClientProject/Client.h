#pragma once

#include <iostream>
#include <string>
#include <WS2tcpip.h>
#include "Logger.h"

#pragma comment (lib, "ws2_32.lib")

using std::cout;
using std::endl;
using std::string;

class Client
{
public:
	Client();
	~Client();

	void startPathDesign();
	void sendRequestToServer();
	void handshakewithServer();

	void receiveResponseFromServer();
	void formatResponseFromServer();

	void handshakeWithNode(int nodeID);

	void sendKeys(SOCKET clientSocket, string keysStr);
	string receiveKeys(SOCKET clientSocket);

	void sendECCKeys(SOCKET clientSocket);
	string receiveECCKeys(SOCKET clientSocket);

	void sendAESKey(SOCKET clientSocket);
	string receiveAESKey(SOCKET clientSocket);

	void checkConnectionAliveTimer();


private:
	SOCKET m_clientSocket;
	int connectionAliveSeconds = 0;
	bool desginPath = true; // Will turn false after the first path design, and true when connectionAliveSeconds > 10 minutes

	Logger logger;
};

