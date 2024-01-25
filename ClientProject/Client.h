#pragma once

#include <iostream>
#include <string>
#include <WS2tcpip.h>
#include "Logger.h"
#include "ClientConnection.h"
#include "Constants.h"

#pragma comment (lib, "ws2_32.lib")

using std::cout;
using std::endl;
using std::string;

using namespace Constants;

class Client
{
public:
	Client();
	~Client();

	void handshakeServer();
	void sendRequestToServer();
	void receiveResponseFromServer();
	void formatResponseFromServer();

	void startPathDesign();
	void handshakeWithNode(int nodePort);

	void checkConnectionAliveTimer();


private:
	ClientConnection clientConnection;
	int connectionAliveSeconds = 0;
	bool desginPath = true; // Will turn false after the first path design, and true when connectionAliveSeconds > 10 minutes

	Logger logger;
};

