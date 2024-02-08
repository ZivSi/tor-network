#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <WS2tcpip.h>
#include "Logger.h"
#include "ClientConnection.h"
#include "Constants.h"
#include "NodeData.h"
#include "RelayObject.h";

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

	void waitForNodes();

	void receiveResponseFromServer();

	void startPathDesign();
	void handshakeWithCurrentPath();
	void handshakeWithNode(unsigned short nodePort, unsigned int nodeIndex);

	void checkConnectionAliveTimer();

	void sendData(string ip, unsigned short port, string message);
	string encrypt(string data);

	void printNodes();

private:
	ECCHandler eccHandler;

	ClientConnection clientConnection;
	int connectionAliveSeconds = 0;
	bool desginPath = true; // Will turn false after the first path design, and true when connectionAliveSeconds > 10 minutes

	Logger logger;

	vector<unsigned short> receivedPorts;
	vector<RelayObject*> currentPath;

	void clearCurrentPath();
	void printPath();
};

