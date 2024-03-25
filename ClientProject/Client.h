#pragma once

#include "../NodeProject/JsonResponse.h"
#include "../ServerProject/ClientConnection.h"
#include "../ServerProject/Constants.h"
#include "../ServerProject/Logger.h"
#include "../ServerProject/NodeData.h"
#include "../ServerProject\ConversationObject.h"
#include "../ServerProject\IConnection.h"
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
using std::thread;

using namespace Constants;

/*
* This class will act as a server for the electron client.
*/

class Client : public IConnection
{
public:
	// ----------------- Constructors & Destructor -----------------
	Client();
	~Client();

	// ----------------- Client Control -----------------
	void stopClient();

	// ----------------- Connection Handling -----------------
	void acceptSocket(SOCKET socket) override;
	void handleClient(SOCKET clientSocket) override;

	void waitForNodes();
	void receiveResponseFromServer();
	ClientConnection* connectToEntryNode();

	// ----------------- Path Design & Handshake -----------------
	void startPathDesign();
	void handshakeWithCurrentPath();
	void handshakeWithNode(string ip, unsigned short nodePort, unsigned int nodeIndex);

	bool isPathDesignCommand(string command);
	int extractPathLength(string command);
	string pathToString();

	// ----------------- Connection Alive Check -----------------
	void checkConnectionAliveTimer();

	// ----------------- Data Transmission -----------------
	void sendData(string ip, unsigned short port, string message, ClientConnection* entryNodeConnection);
	void sendData(string username, string message, ClientConnection* entryNodeConnection);
	void sendData(DestinationData dd, ClientConnection* entryNodeConnection);

	// ----------------- Electron Communication -----------------
	void informElectron(SOCKET socket, const string& message);
	void passResponseToElectron(SOCKET socket, const string& response);
	void sendErrorToElectron(SOCKET socket, int errorType, const string& message);

	void receiveInLoopToElectron(SOCKET electronSocket, ClientConnection** entryNodeConnection);

	// ----------------- Encryption & Decryption -----------------
	string encrypt(string ip, unsigned short port, string message);
	string encrypt(string username, string message);
	string decrypt(string encrypted);

	// ----------------- Utility -----------------
	void printNodes();
	unsigned long long getConnectionTime();
	bool isErrorResponse(string& reponseString);

private:
	// ----------------- Member Variables -----------------
	ECCHandler eccHandler;
	unsigned long long currentPathAliveTime = 0;
	Logger logger;
	ClientConnection clientConnection;
	int connectionAliveSeconds = 0;
	bool desginPath = true;
	vector<RelayProperties> receivedRelays;
	vector<RelayObject*> currentPath;
	mutex currentPathMutex;

	// ----------------- Private Helper Methods -----------------
	void clearCurrentPath();
	void printPath();
	bool pathAvailable();
	bool pathIsTooOld();

	// ----------------- Control Variables -----------------
	bool stop = false;
};
