#pragma once

#include "../ServerProject/ClientConnection.h"
#include "../ServerProject/Constants.h"
#include "../ServerProject/Logger.h"
#include "../ServerProject/NodeData.h"
#include "RelayObject.h";
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include <WS2tcpip.h>

#pragma comment (lib, "ws2_32.lib")

using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::mutex;


using namespace Constants;

class Client
{
public:
	Client();
	~Client();

	void stopClient();

	void waitForNodes();

	void receiveResponseFromServer();

	void startPathDesign();
	void handshakeWithCurrentPath();
	void handshakeWithNode(string ip, unsigned short nodePort, unsigned int nodeIndex);

	void checkConnectionAliveTimer();

	ClientConnection* connectToEntryNode();
	void sendData(string ip, unsigned short port, string message, ClientConnection* entryNodeConnection); // Call connectToEntryNode() before this
	string encrypt(string ip, unsigned short port, string data);
	string decrypt(string encrypted);

	void printNodes();

	unsigned long long getConnectionTime();

private:
	ECCHandler eccHandler;
	unsigned long long currentPathAliveTime = 0;

	Logger logger;


	ClientConnection clientConnection;
	int connectionAliveSeconds = 0;
	bool desginPath = true; // Will turn false after the first path design, and true when connectionAliveSeconds > 10 minutes

	vector<RelayProperties> receivedRelays;
	vector<RelayObject*> currentPath;
	mutex currentPathMutex;

	void clearCurrentPath();
	void printPath();
	bool pathAvailable();
	bool pathIsTooOld();

	bool stop = true;
};

