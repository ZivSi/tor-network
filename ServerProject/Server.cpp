#include "Server.h"

Server::Server() : logger("Server"), IConnection(LOCALHOST, SERVER_PORT, &logger), stop(true) {
}

void Server::startServer() {
	stop = false;

	logger.log("The parent server is running on " + SERVER_IP + ":" + to_string(Constants::SERVER_PORT));

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

	this->aliveNodes.clear();
	aliveNodesMutex.unlock();
}


void Server::acceptSocket(SOCKET socket) {
	sockaddr_in client;
	int clientSize = sizeof(client);
	SOCKET clientSocket;

	try {
		while (!stop) {

			// Move clientSocket declaration outside the loop
			clientSocket = accept(socket, (sockaddr*)&client, &clientSize);

			if (clientSocket == INVALID_SOCKET) {
				int error = WSAGetLastError();
				if (error != WSAEWOULDBLOCK) {
					logger.error("Can't accept client socket, Error: " + std::to_string(error));
				}

				continue;
			}

			logger.log("Client connected");

			std::thread clientThread(&Server::handleClient, this, clientSocket);
			clientThread.detach();
		}
	}
	catch (const std::exception& e) {
		logger.error("Error in acceptSocket(): " + std::string(e.what()));
	}
	catch (...) {
		logger.error("Unknown error in acceptSocket()");
	}
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
		sendECCKey(clientSocket);

		string receivedECCKeys = this->receiveECCKeys(clientSocket);
		logger.keysInfo("Received ECC keys from client: " + Utility::asHex(receivedECCKeys));


		string receivedAESKeys;
		try {
			receivedAESKeys = this->receiveAESKey(clientSocket);
			logger.keysInfo("Received AES keys from client");
		}
		catch (Exception) {
			logger.error("Error in receiving AES keys");

			closesocket(clientSocket);
			return;
		}

		string decryptedAESKeys;

		try {
			decryptedAESKeys = decryptECC(receivedAESKeys);

			logger.keysInfo("Decrypted AES keys from client: " + Utility::asHex(decryptedAESKeys));
		}
		catch (std::runtime_error e) {
			logger.error("Couldn't decrypt AES keys from client. Closing connection");
			closesocket(clientSocket);
			return;
		}


		AesKey temp = AesKey::decryptedAESKeysToPair(decryptedAESKeys);

		string received = receiveData(clientSocket);

		if (received.empty()) {
			logger.log("Received empty data from client. Closing connection");

			closesocket(clientSocket);

			return;
		}

		string decrypted;
		try {
			decrypted = AesHandler::decryptAES(received, &temp);
			logger.log("Decrypted data from client: " + decrypted);
		}
		catch (std::runtime_error e) {
			logger.error("Error in decrypting data from client");

			closesocket(clientSocket);
			return;
		}

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

		string ip = parts[IP];
		unsigned short port = static_cast<unsigned short>(stoi(parts[PORT]));


		NodeData* node = getNodeInVector(ip, port);

		if (node == NULL) {

			aliveNodesMutex.lock();
			this->aliveNodes.push_back(new NodeData(ip, port, receivedECCKeys, Utility::capture_time(), 1, 0));
			aliveNodesMutex.unlock();

			closesocket(clientSocket);

			logger.success("New node added to alive nodes list: " + to_string(port));

			return;
		}

		node->updateLastAliveMessageTime();

		logger.success("Updated node's last alive for: " + ip + ":" + to_string(port));

		closesocket(clientSocket);
	}
	catch (std::runtime_error e) {
		aliveNodesMutex.unlock();

		logger.error("Can't handle connection. Closing...");
		logger.error(e.what());

		closesocket(clientSocket);
	}
}

bool Server::isNode(string data) {
	int currentSplitSize = static_cast<int>(Utility::splitString(data, SPLITER).size());

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
	string textToHash = parts[IP] + SPLITER + parts[PORT] + SPLITER + parts[ECC_PUBLIC_KEY] + SPLITER + parts[CURRENT_TIME] + SPLITER + parts[RANDOM_NON_PRIME_NUMBER] + SPLITER + parts[RANDOM_NUMBER] + SPLITER + parts[CONDITION_NUMBER];
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

	vector<RelayProperties> portsStream;

	for (NodeData* node : this->aliveNodes) {
		string ip = node->getIp();
		unsigned short port = node->getPort();
		portsStream.push_back(RelayProperties(ip, port));
	}

	aliveNodesMutex.unlock();

	if (portsStream.empty()) {
		logger.log("No nodes to send");

		sendData(clientSocket, NO_NOEDS_REPONSE);

		return;
	}

	string serializedData(reinterpret_cast<const char*>(portsStream.data()), portsStream.size() * sizeof(RelayProperties));

	sendData(clientSocket, serializedData);
}

NodeData* Server::getNodeInVector(string ip, unsigned short port) {
	aliveNodesMutex.lock();
	for (NodeData* node : this->aliveNodes) {
		if (node->getIp() == ip && node->getPort() == port) {
			aliveNodesMutex.unlock();

			return node;
		}
	}

	aliveNodesMutex.unlock();

	return NULL;
}

void Server::checkAliveNodes()
{
	logger.log("Checking alive nodes..");

	while (!stop) {
		try {
			aliveNodesMutex.lock();

			for (int i = 0; i < this->aliveNodes.size(); i++) {
				NodeData* node = aliveNodes[i];

				if (nodeIsDead(node)) {

					logger.error("Node " + node->getIp() + ":" + to_string(node->getPort()) + " is dead");
					delete node;

					this->aliveNodes.erase(this->aliveNodes.begin() + i);

					printNodes();
				}
			}

			aliveNodesMutex.unlock();
			Sleep(1000);
		}
		catch (...) {
			aliveNodesMutex.unlock();

			logger.error("Error in checkAliveNodes()");
		}
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

	if (this->aliveNodes.empty()) {
		cout << "No nodes" << endl;
	}
}

void Server::initializeNodes(vector<NodeData*> nodes)
{
	aliveNodesMutex.lock();
	this->aliveNodes = nodes;
	aliveNodesMutex.unlock();
}

bool Server::nodeIsDead(NodeData* node)
{
	return Utility::capture_time() - node->getLastAliveMessageTime() > Constants::MAX_TIME_ALIVE;
}