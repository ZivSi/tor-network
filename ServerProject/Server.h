#pragma once

#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <thread>
#include "NodeData.h"
#include "AesHandler.h"
#include "RSAHandler.h"
#include "ECCHandler.h"
#include "Utility.h"
#include "Constants.h"
#include <mutex>
#include <secblock.h>
#include <cryptlib.h>
#include <eccrypto.h>
#include <stdexcept>
#include <oids.h>
#include "Logger.h"
#include <random>
#include <WinSock2.h>
#include <winsock2.h>
#include <WS2tcpip.h>
#include "IConnection.h"


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

	mutex eccHandlerMutex;
	ECCHandler eccHandler;

	mutex aliveNodesMutex;
	vector<NodeData*> aliveNodes;

	mutex nodesMutex;
	Logger logger;

	mutex emptyNodeMutex;
	NodeData EMPTY_NODE;

	string decrypt(string encrypted);

	void sendECCKeys(SOCKET clientSocket);
	string receiveECCKeys(SOCKET clientSocket);
	string receiveAESKey(SOCKET clientSocket);

	bool isNode(string decrypted);

	// If true
	bool isValidFormat(string decrypted);

	// If not
	void sendNodesToClient(SOCKET clientSocket);

	NodeData* getNodeInVector(unsigned short port);
	void checkAliveNodes();
};