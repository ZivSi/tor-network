#pragma once

#include "../ClientProject/RelayObject.h"
#include "AesHandler.h"
#include "Constants.h"
#include "ECCHandler.h"
#include "IConnection.h"
#include "Logger.h"
#include "NodeData.h"
#include "RSAHandler.h"
#include "Utility.h"
#include <cryptlib.h>
#include <eccrypto.h>
#include <iostream>
#include <mutex>
#include <oids.h>
#include <random>
#include <secblock.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>
#include <WinSock2.h>
#include <winsock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

using std::string;
using std::vector;
using std::cout;
using std::cerr;
using std::endl;
using std::thread;
using std::to_string;
using std::mutex;

using namespace Constants;

class Server : public IConnection {
public:
	Server();
	~Server();

	void startServer();
	void stopServer();

	void printNodes();
	void initializeNodes(vector<NodeData*> nodes);

	void acceptSocket(SOCKET socket) override;
	void handleClient(SOCKET clientSocket) override;

private:
	bool stop;

	mutex aliveNodesMutex;
	vector<NodeData*> aliveNodes;

	mutex nodesMutex;
	Logger logger;

	string receiveECCKeys(SOCKET clientSocket);
	string receiveAESKey(SOCKET clientSocket);

	bool isNode(string decrypted);
	bool nodeIsDead(NodeData* node);

	// If true
	bool isValidFormat(string decrypted);

	// If not
	void sendNodesToClient(SOCKET clientSocket);

	NodeData* getNodeInVector(string ip, unsigned short port);
	void checkAliveNodes();
};