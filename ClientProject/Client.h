#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <WS2tcpip.h>
#include "Logger.h"
#include "ClientConnection.h"
#include "Constants.h"

#pragma comment (lib, "ws2_32.lib")

using std::cout;
using std::endl;
using std::string;
using std::vector;

using namespace Constants;

class Client
{
public:
	Client();
	~Client();

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

	vector<unsigned short> receivedPorts;
};

