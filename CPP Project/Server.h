#pragma once

#include <string>
#include <iostream>
#include <vector>
#include <thread>
#include "ClientData.hpp"
#include "NodeData.hpp"
#include "AesHandler.h"
#include "RSAHandler.h"
#include "Utility.hpp"
#include <mutex>
#include "DataObject.hpp"

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

class Server {
public:
	Server();
	~Server();

	void startServer();
	void stopServer();

private:
	RSAHandler rsaObject;
	mutex rsaObjectMutex;

	AesHandler aesObject;
	mutex aesObjectMutex;

	string RSA_KEYS_FORMAT;
	mutex RSA_KEYS_FORMATMutex;

	vector<ClientData> clients;
	vector<NodeData> nodes;

	mutex clientsVectorMutex;
	mutex nodesVectorMutex;

	SOCKET serverSocket;
	mutex serverSocketMutex;

	bool stop = false;
	mutex stopMutex;

	SOCKET initWSASocket();
	void bindSocket(SOCKET socket);
	void listenSocket(SOCKET socket);
	void acceptSocket(SOCKET socket);

	void sendData(SOCKET socket, string data);
	string receiveData(SOCKET socket);
	string receiveDataWithTimeout(SOCKET socket, int timeoutInSeconds);

	void sendRSAKeys(SOCKET socket);

	void handleClient(ClientData clientData);
	void handleNode(NodeData* nodeData);

	bool determine(string decryptedResponse); // True if node, false if client
	int getPortFrom(SOCKET socket);
	string getIPFrom(SOCKET socket);

	NodeData extractNodeData(string decryptedResponse);
	ClientData extractClientData(string decryptedResponse);

	SOCKET connectToServer(string serverIP, int serverPort);
	string encrypt(string data, vector<NodeData> nodes);
	void sendToFirstNode(string data);

	void checkNodes();
	bool isNodeAlive(NodeData node);
	bool nodeInVector(NodeData node);
};