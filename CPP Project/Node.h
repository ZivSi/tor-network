#pragma once

#include "Constants.h"
#include "AesHandler.h"
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include "Utility.hpp"

#include <WS2tcpip.h>
#include <WinSock2.h>

#pragma comment(lib, "Ws2_32.lib")

using std::string;
using std::vector;
using std::cout;
using std::cerr;
using std::endl;
using std::thread;
using std::to_string;

class Node
{
private:
	AesHandler aesObject;

	SOCKET myServerSocket;
	int myPort;

	unsigned long long parentPublicKey;
	unsigned long long parentModulus;

	bool firstMessage = true;
	bool receivedParentPublicKey = false;

	string aliveFormat;

	SOCKET initWSASocket();

	void sendData(SOCKET socket, string data);

	void bindSocket(SOCKET socket);
	void acceptInThread(SOCKET socket);
	void listenSocket(SOCKET socket);
	void acceptSocket(SOCKET socket);

	int getPortFrom(SOCKET socket);
	string getIPFrom(SOCKET socket);

	SOCKET connectToServer(string serverIP, int serverPort);

	void sendAliveMessage(SOCKET socket);
	vector<string> extractData(string decrypted);

	string receiveData(SOCKET socket);

	bool startWithPort(string decrypted);
	bool reachedDestination(string decrypted);
	void sendToNextNode(string decrypted, int port, string data);
	bool isPortTaken(int port);

public:
	Node();
	~Node();

	void main();
};

