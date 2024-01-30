#include "Server.h"

Server::Server() : logger("Server"), IConnection("127.0.0.1", SERVER_PORT, &logger) {
}

void Server::startServer() {
	logger.log("Server is listening on port " + to_string(Constants::SERVER_PORT));

	thread acceptInThread(&Server::acceptSocket, this, getSocket());
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
	closeConnection();

	this->stop = true;

	cout << "Server closed" << endl;

	aliveNodesMutex.lock();
	for (NodeData* node : this->aliveNodes) {
		delete node;
	}
	aliveNodesMutex.unlock();

	this->aliveNodes.clear();
}


void Server::acceptSocket(SOCKET socket) {
	try {
		while (!stop) {
			sockaddr_in client;
			int clientSize = sizeof(client);

			SOCKET clientSocket = accept(socket, (sockaddr*)&client, &clientSize);

			if (clientSocket == INVALID_SOCKET) {
				logger.error("Can't accept client socket, Err #" + WSAGetLastError());
				exit(1);
			}

			logger.log("Client connected");

			thread clientThread(&Server::handleClient, this, clientSocket);
			clientThread.detach();
		}
	}
	catch (...) {
		logger.error("Error in acceptSocket()");
	}
}

void Server::sendECCKeys(SOCKET clientSocket)
{
	eccHandlerMutex.lock();
	string keysStr = eccHandler.serializeKey();
	eccHandlerMutex.unlock();

	sendKeys(clientSocket, keysStr);

	logger.log("Sent public key");
}


string Server::receiveECCKeys(SOCKET clientSocket)
{
	return receiveKeys(clientSocket);
}


string Server::receiveAESKey(SOCKET clientSocket) {
	return receiveKeys(clientSocket);
}

void Server::handleClient(SOCKET clientSocket)
{
	try {
		this->sendECCKeys(clientSocket);
		logger.keysInfo("Sent ECC keys to the client");


		string receivedECCKeys = this->receiveECCKeys(clientSocket);
		logger.keysInfo("Received ECC keys from client: " + Utility::asHex(receivedECCKeys));


		string receivedAESKeys;
		try {
			receivedAESKeys = this->receiveAESKey(clientSocket);
			logger.keysInfo("Received AES keys from client");
		}
		catch (Exception) {
			logger.error("Error in receiving AES keys");
			return;
		}

		string decryptedAESKeys;
		try {
			eccHandlerMutex.lock();
			decryptedAESKeys = eccHandler.decrypt(receivedAESKeys); // If client, we will get something like "Hi from client"
			eccHandlerMutex.unlock();

			logger.keysInfo("Decrypted AES keys from client: " + Utility::asHex(decryptedAESKeys));
		}
		catch (Exception e) {
			eccHandlerMutex.unlock();

			logger.error("Error in eccHandler.decrypt");
			cout << e.what() << endl << "Input was: " << receivedAESKeys << endl;

			return;
		}


		string extractedAes = "";
		string extractedIv = "";

		Utility::extractAESKey(decryptedAESKeys, extractedAes);
		Utility::extractAESIv(decryptedAESKeys, extractedIv);

		SecByteBlock aesKey = AesHandler::StringToSecByteBlock(extractedAes);
		SecByteBlock aesIv = AesHandler::StringToSecByteBlock(extractedIv);

		logger.keysInfo("Extracted AES key and IV from received data");

		string received = receiveData(clientSocket);

		if (received.empty()) {
			logger.log("Received empty data from client. Closing connection");

			closesocket(clientSocket);

			return;
		}

		string decrypted = AesHandler::decryptAES(received, aesKey, aesIv);
		logger.log("Decrypted data from client: " + decrypted);

		if (!isNode(decrypted)) {
			logger.success("Client authorized");
			sendNodesToClient(clientSocket);

			closesocket(clientSocket);

			return;
		}

		if (!isValidFormat(decrypted)) {
			closesocket(clientSocket);

			return;
		}

		logger.success("Node's format is valid");

		vector<string> parts = Utility::splitString(decrypted, SPLITER);

		unsigned short port = static_cast<unsigned short>(stoi(parts[0]));

		NodeData* node = getNodeInVector(port);

		if (node->isEmpty()) {
			delete node;

			aliveNodesMutex.lock();
			this->aliveNodes.push_back(new NodeData(port, receivedECCKeys, Utility::capture_time(), 1, 0));
			aliveNodesMutex.unlock();

			closesocket(clientSocket);

			logger.success("New node added to alive nodes list: " + to_string(port));

			return;
		}

		node->updateLastAliveMessageTime();

		logger.success("Updated node's last alive time: " + to_string(port));
	}
	catch (...) {
		aliveNodesMutex.unlock();

		logger.error("Error in handleConnection(). Quitting");

		closesocket(clientSocket);
	}
}


string Server::decrypt(string encrypted) {
	return eccHandler.decrypt(encrypted);
}

bool Server::isNode(string data) {
	int currentSplitSize = Utility::splitString(data, SPLITER).size();

	return currentSplitSize == NODE_SPLIT_SIZE;
}

bool Server::isValidFormat(string data) {
	vector<string> parts = Utility::splitString(data, SPLITER);

	if (!Utility::isNumber(parts[PORT])) {
		logger.formatError("Port is not a number");
		return false;
	}

	unsigned short port = static_cast<unsigned short>(stoi(parts[PORT]));
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
	aliveNodesMutex.lock();
	logger.log("Passing alive nodes to the client (" + to_string(aliveNodes.size()) + ")");

	vector<unsigned short> portsStream;

	for (NodeData* node : this->aliveNodes) {
		unsigned short port = node->getPort();
		portsStream.push_back(port);
	}
	aliveNodesMutex.unlock();

	if (portsStream.empty()) {
		logger.log("No nodes to send");

		sendData(clientSocket, NO_NOEDS_REPONSE);

		return;
	}

	// Devide the portsStream into a string (bytes of data / sizeof(unsigned short))
	string serializedData(reinterpret_cast<const char*>(portsStream.data()), portsStream.size() * sizeof(unsigned short));

	sendData(clientSocket, serializedData);
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

	return new NodeData();
}

void Server::checkAliveNodes()
{
	try {
		logger.log("Checking alive nodes..");
		while (true) {
			aliveNodesMutex.lock();

			for (int i = 0; i < this->aliveNodes.size(); i++) {
				NodeData* node = aliveNodes[i];

				if (Utility::capture_time() - node->getLastAliveMessageTime() > Constants::MAX_TIME_ALIVE) {

					logger.error("Node " + to_string(node->getPort()) + " is dead");
					delete node;

					this->aliveNodes.erase(this->aliveNodes.begin() + i);

					printNodes();
				}
			}

			aliveNodesMutex.unlock();
			Sleep(1000);
		}
	}
	catch (...) {
		aliveNodesMutex.unlock();

		logger.error("Error in checkAliveNodes()");

		checkAliveNodes();
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