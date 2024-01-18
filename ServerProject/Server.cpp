#include "Server.h"

Server::Server() : logger("Server") {
}

void Server::startServer() {
	this->serverSocket = initWSASocket();
	bindSocket(this->serverSocket);

	logger.log("Server is bound to port " + to_string(Constants::SERVER_PORT));

	listenSocket(this->serverSocket);

	logger.log("Server is listening on port " + to_string(Constants::SERVER_PORT));

	thread acceptInThread(&Server::acceptSocket, this, this->serverSocket);
	acceptInThread.detach();

	logger.log("Server is accepting clients");

	thread checkAliveNodesThread(&Server::checkAliveNodes, this);
	checkAliveNodesThread.detach();

	logger.log("Server is checking alive nodes");
}

void Server::stopServer() {
	this->~Server();
}

Server::~Server() {
	this->stop = true;

	closesocket(this->serverSocket);
	WSACleanup();

	cout << "Server closed" << endl;
}

SOCKET Server::initWSASocket() {
	WSADATA wsaData;
	WORD version = MAKEWORD(2, 2);
	int wsaResult = WSAStartup(version, &wsaData);

	if (wsaResult != 0) {
		cerr << "Can't start Winsock, Err #" << wsaResult << endl;
		exit(1);
	}

	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
	if (listening == INVALID_SOCKET) {
		cerr << "Can't create a socket, Err #" << WSAGetLastError() << endl;
		exit(1);
	}

	return listening;
}

void Server::bindSocket(SOCKET socket) {
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(Constants::SERVER_PORT);
	hint.sin_addr.S_un.S_addr = INADDR_ANY;

	bind(socket, (sockaddr*)&hint, sizeof(hint));
}

void Server::listenSocket(SOCKET socket) {
	if (socket == INVALID_SOCKET) {
		logger.error("Can't create a socket, Err #" + WSAGetLastError());
		exit(1);
	}

	int listening = listen(socket, SOMAXCONN);
	if (listening == SOCKET_ERROR) {
		logger.error("Can't listen on socket, Err #" + WSAGetLastError());
		exit(1);
	}
}

void Server::acceptSocket(SOCKET socket) {
	while (!stop) {
		sockaddr_in client;
		int clientSize = sizeof(client);

		SOCKET clientSocket = accept(socket, (sockaddr*)&client, &clientSize);

		if (clientSocket == INVALID_SOCKET) {
			logger.error("Can't accept client socket, Err #" + WSAGetLastError());
			exit(1);
		}

		logger.log("Client connected");

		thread clientThread(&Server::handleConnection, this, clientSocket);
		clientThread.detach();
	}
}

void Server::sendECCKeys(SOCKET clientSocket)
{
	string keysStr = eccHandler.serializeKey();
	int dataSize = keysStr.size();

	send(clientSocket, reinterpret_cast<const char*>(&dataSize), sizeof(size_t), 0);

	send(clientSocket, keysStr.data(), dataSize, 0);

	logger.log("Sent public key");
}

string Server::receiveKeys(SOCKET clientSocket)
{
	int dataSize = 0;

	// First, receive the size of the data
	recv(clientSocket, reinterpret_cast<char*>(&dataSize), sizeof(size_t), 0);

	// Allocate a buffer to store the received data
	char* buffer = new char[dataSize];

	// Receive the key
	recv(clientSocket, buffer, dataSize, 0);

	// Create a string from the received data
	string keysStr(buffer, dataSize);

	delete[] buffer;


	return keysStr;
}

string Server::receiveECCKeys(SOCKET clientSocket)
{
	return receiveKeys(clientSocket);
}

string Server::receiveAESKey(SOCKET clientSocket) {
	return receiveKeys(clientSocket);
}

void Server::handleConnection(SOCKET clientSocket)
{
	try {

		this->sendECCKeys(clientSocket);
		logger.keysInfo("Sent ECC keys to the client");


		string eccKeys = this->receiveECCKeys(clientSocket);
		logger.keysInfo("Received ECC keys from client: " + eccKeys);


		string receivedAES;
		try {
			receivedAES = this->receiveAESKey(clientSocket);
			logger.keysInfo("Received AES keys from client");
		}
		catch (Exception) {
			logger.error("Error in receiving AES keys");
			return;
		}

		string aesKeys;
		try {
			aesKeys = eccHandler.decrypt(receivedAES);
		}
		catch (Exception e) {
			logger.error("Error in eccHandler.decrypt");
			cout << e.what() << endl;
			return;
		}


		string extractedAes = "";
		string extractedIv = "";

		Utility::extractAESKey(aesKeys, extractedAes);
		Utility::extractAESIv(aesKeys, extractedIv);

		SecByteBlock aesKey = AesHandler::StringToSecByteBlock(extractedAes);
		SecByteBlock aesIv = AesHandler::StringToSecByteBlock(extractedIv);

		logger.keysInfo("Extracted AES key and IV from received data");

		string received = receiveData(clientSocket);


		string decrypted = AesHandler::decryptAES(received, aesKey, aesIv);
		logger.log("Decrypted data from client: " + decrypted);

		if (!isNode(decrypted)) {
			sendNodesToClient(clientSocket);

			closesocket(clientSocket);
			return;
		}

		if (!isValidFormat(decrypted)) {
			closesocket(clientSocket);

			return;
		}

		logger.success("Node's format is valid!");

		vector<string> parts = Utility::splitString(decrypted, SPLITER);
		unsigned short port = stoi(parts[0]);
		// string eccPublicKey = parts[1]; we don't need it, we already have it

		NodeData* node = getNodeInVector(port);

		if (*node == EMPTY_NODE) {
			aliveNodesMutex.lock();
			this->aliveNodes.push_back(new NodeData(port, eccKeys, Utility::capture_time(), 1, 0));
			aliveNodesMutex.unlock();

			closesocket(clientSocket);

			logger.success("New node added to alive nodes list");

			return;
		}

		node->updateLastAliveMessageTime();

		logger.success("Updated node's last alive time");
	}
	catch (Exception) {
		logger.error("Error in handleConnection(). Quitting");

		closesocket(clientSocket);
		return;
	}
}

void Server::sendData(SOCKET clientSocket, string data) {
	size_t sendResult = send(clientSocket, data.c_str(), data.size() + 1, 0);

	if (sendResult == SOCKET_ERROR) {
		cerr << "Can't send data to client, Err #" << WSAGetLastError() << endl;
		exit(1);
	}
}

string Server::receiveData(SOCKET clientSocket)
{
	size_t dataSize = 0;

	// First, receive the size of the data
	recv(clientSocket, reinterpret_cast<char*>(&dataSize), sizeof(size_t), 0);

	// Allocate a buffer to store the received data
	char* buffer = new char[dataSize];

	// Receive the key
	recv(clientSocket, buffer, dataSize, 0);

	// Create a string from the received data
	string data(buffer, dataSize);

	delete[] buffer;


	return data;

}

string Server::decrypt(string encrypted) {
	return eccHandler.decrypt(encrypted);
}

bool Server::isNode(string data) {
	int currentSplitSize = Utility::splitString(data, SPLITER).size();
	logger.log("Current size: " + to_string(currentSplitSize));
	logger.log("Original size: " + to_string(NODE_SPLIT_SIZE));

	return currentSplitSize == NODE_SPLIT_SIZE;
}

bool Server::isValidFormat(string data) {
	vector<string> parts = Utility::splitString(data, SPLITER);

	if (!Utility::isNumber(parts[PORT])) {
		logger.formatError("Port is not a number");
		return false;
	}

	unsigned short port = stoi(parts[PORT]);
	string eccPublicKey = parts[ECC_PUBLIC_KEY];

	if (!Utility::isValidECCPublicKey(eccPublicKey)) {
		logger.formatError("Not valid ECC key");
		return false;
	}

	if (!Utility::isValidTime(parts[CURRENT_TIME])) {
		logger.formatError("Time is not valid");
		return false;
	}

	if (!Utility::isNumber(parts[RANDOM_NON_PRIME_NUMBER])) {
		logger.formatError("Random non-prime is not a number");
		return false;
	}

	long nonPrime = stol(parts[RANDOM_NON_PRIME_NUMBER]);

	if (Utility::isPrime(nonPrime)) {
		logger.formatError("Random non-prime is prime");
		return false;
	}

	if (!Utility::isNumber(parts[RANDOM_NUMBER])) {
		logger.formatError("Random number is not a number");
		return false;
	}

	if (!Utility::isNumber(parts[CONDITION_NUMBER])) {
		logger.formatError("Condition number is not a number");
		return false;
	}

	long condition = stol(parts[CONDITION_NUMBER]);

	bool modulusValid = nonPrime % (condition - port) == 1;

	if (!modulusValid) {
		logger.formatError("Modulus is not valid!");
		return false;
	}

	// Hash part
	string textToHash = parts[PORT] + SPLITER + parts[ECC_PUBLIC_KEY] + SPLITER + parts[CURRENT_TIME] + SPLITER + parts[RANDOM_NON_PRIME_NUMBER] + SPLITER + parts[RANDOM_NUMBER] + SPLITER + parts[CONDITION_NUMBER];
	string smallHash = Utility::hashStr(textToHash + Constants::PEPPER);

	if (smallHash != parts[HASH]) {
		logger.formatError("Small hash is not valid");
		return false;
	}

	// Extended hash part
	textToHash = textToHash + SPLITER + parts[HASH];
	string extendedHash = Utility::hashStr(textToHash + Constants::PEPPER2);

	if (extendedHash != parts[EXTENDED_HASH]) {
		logger.formatError("Extended hash is not valid");
		return false;
	}

	return true;
}

void Server::sendNodesToClient(SOCKET clientSocket)
{
	vector<unsigned short> portsStream;


	// We represent the number of nodes as 2 bytes (unsigned short) becuase ports can be up to 65535
	aliveNodesMutex.lock();
	for (NodeData* node : this->aliveNodes) {
		unsigned short port = node->getPort();

		portsStream.push_back(port);
	}

	aliveNodesMutex.unlock();

	// Send the ports to the client
	sendData(clientSocket, string(portsStream.begin(), portsStream.end()));
}

NodeData* Server::getNodeInVector(unsigned short port) {
	aliveNodesMutex.lock();
	for (NodeData* node : this->aliveNodes) {
		if (node->getPort() == port) {
			aliveNodesMutex.unlock();

			return node;
		}
	}

	aliveNodesMutex.unlock();

	return &EMPTY_NODE;
}

void Server::checkAliveNodes()
{
	while (!stop) {
		aliveNodesMutex.lock();

		for (int i = 0; i < this->aliveNodes.size(); i++) {
			NodeData* node = aliveNodes[i];

			if (Utility::capture_time() - node->getLastAliveMessageTime() > Constants::MAX_TIME_ALIVE) {
				this->aliveNodes.erase(this->aliveNodes.begin() + i);
			}
		}

		aliveNodesMutex.unlock();
		Sleep(400 + (40 * aliveNodes.size())); // More nodes - longer the mutex is locked, so when it does open, allow more time to be unlocked
	}
}

void Server::printNodes()
{
	cout << "--------------------------------------------" << endl;
	aliveNodesMutex.lock();
	cout << "Number of nodes: " << this->aliveNodes.size() << endl;
	cout << "Nodes list:" << endl;

	for (NodeData* node : this->aliveNodes) {
		cout << node->toString() << endl;
	}

	aliveNodesMutex.unlock();

	cout << "--------------------------------------------" << endl;
}

void Server::initializeNodes(vector<NodeData*> nodes)
{
	aliveNodesMutex.lock();
	this->aliveNodes = nodes;
	aliveNodesMutex.unlock();
}