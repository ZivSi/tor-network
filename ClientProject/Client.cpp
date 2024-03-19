#include "Client.h"

Client::Client() : logger("Client"), clientConnection("127.0.0.1", SERVER_PORT, logger), stop(false), IConnection(LOCALHOST, LOCAL_CLIENT_PORT, &logger)
{
	clientConnection.handshake();
	clientConnection.sendEncrypted("Hi. I'm a client"); // Initial message

	thread acceptInThread(&Client::acceptSocket, this, getSocket());
	acceptInThread.detach();
}

Client::~Client() {
	closeConnection();

	clientConnection.closeConnection();

	stop = true;

	clearCurrentPath();

	receivedRelays.clear();

	logger.log("Client destroyed");
}

void Client::stopClient()
{
	this->~Client();
}

void Client::acceptSocket(SOCKET socket) {
	sockaddr_in client{};
	int clientSize = sizeof(client);
	SOCKET clientSocket;

	logger.log("Waiting for connections...");

	while (!stop) {
		clientSocket = accept(socket, (sockaddr*)&client, &clientSize);

		if (clientSocket == INVALID_SOCKET) {
			int error = WSAGetLastError();
			if (error != WSAEWOULDBLOCK) {
				logger.error("Can't accept client socket, Error: " + std::to_string(error));
			}

			continue;
		}

		logger.clientEvent("Accepted connection from electron client");

		std::thread clientThread(&Client::handleClient, this, clientSocket);
		clientThread.detach();
	}
}

void Client::handleClient(SOCKET clientSocket)
{
	// 1. wait for "start path design" message
	// 2. start path design
	// 3. handshake with current path
	// 4. start listening for messages
	// 5. send success message to client with properties
	// 6. wait for messages from the client and send them to the next node

	bool pathDesignComplete = false;
	ClientConnection* entryNodeConnection = nullptr;

	thread receiveThread(&Client::receiveInLoopToElectron, this, clientSocket, &entryNodeConnection);
	receiveThread.detach();

	while (!stop) {
		string received = "";

		try {
			received = this->receiveData(clientSocket);

			if (received.empty()) {
				continue;
			}

			if (isPathDesignCommand(received)) {
				clearCurrentPath();

				int pathLength = extractPathLength(received);

				if (pathLength == -1 || pathLength < DEFAULT_PATH_LENGTH) {
					logger.error("Invalid path length received");

					sendToElectron(clientSocket, "Invalid path length received");

					continue;
				}

				sendToElectron(clientSocket, "Waiting for nodes...");

				waitForNodes();
				receiveResponseFromServer();

				sendToElectron(clientSocket, "Path design starting now...");
				startPathDesign();

				sendToElectron(clientSocket, "Path design completed");
				sendToElectron(clientSocket, "Handshaking with nodes...");

				handshakeWithCurrentPath();

				sendToElectron(clientSocket, "Handshake completed");
				pathDesignComplete = true;

				sendToElectron(clientSocket, "Path design completed");

				sendToElectron(clientSocket, "Path: " + pathToString());

				entryNodeConnection = connectToEntryNode();
			}
			else if (pathIsTooOld()) {
				sendToElectron(clientSocket, "Path is too old. Please start a new path design");
				clearCurrentPath();

				pathDesignComplete = false;
			}
			else if (pathDesignComplete && pathAvailable()) {
				// Send the message to the next node
				if (entryNodeConnection == nullptr || !entryNodeConnection->isConnectionActive()) {
					if (entryNodeConnection == nullptr) { delete entryNodeConnection; }

					entryNodeConnection = connectToEntryNode();
				}

				try {
					DestinationData dd(received);
					sendData(dd, entryNodeConnection);
				}
				catch (...) {
					vector<string> split = Utility::splitString(received, SPLITER);

					if (split.size() == 2) {
						string username = split[0];
						string message = Utility::formatData(split[1]);

						sendData(username, message, entryNodeConnection);
					}
					else {
						sendToElectron(clientSocket, "Invalid input format. Please provide input in the format 'username::::message' or ip::::port::::message");
					}
				}
			}
			else {
				sendToElectron(clientSocket, "Path design not started yet");
			}
		}
		catch (std::exception& e) {
			logger.error("Error in client handling: " + std::string(e.what()));

			clearCurrentPath();

			stop = true;
		}
	}
}

void Client::waitForNodes()
{
	receiveResponseFromServer();

	while (receivedRelays.empty()) {
		clientConnection.closeConnection();

		Sleep(CLIENT_WAIT_FOR_NODES);

		clientConnection.connectInLoop();
		clientConnection.handshake();
		clientConnection.sendEncrypted(CLIENT_INITIAL_MESSAGE);

		receiveResponseFromServer();
	}
}

void Client::receiveResponseFromServer()
{
	string received = clientConnection.receiveData();
	cout << "Received from server: " << received << endl;

	if (received.empty() || received == NO_NOEDS_REPONSE) {
		logger.error("No nodes available");
		return;
	}

	receivedRelays.clear();

	// Put data inside receivedRelays
	const RelayProperties* relayData = reinterpret_cast<const RelayProperties*>(received.data());
	size_t relayCount = received.size() / sizeof(RelayProperties);

	// Populate the receivedRelays vector
	for (size_t i = 0; i < relayCount; ++i) {
		receivedRelays.push_back(relayData[i]);
	}

	cout << "Received " << receivedRelays.size() << " nodes" << endl;
	for (RelayProperties relay : receivedRelays) {
		cout << relay.getIp() << ":" << relay.getPort() << endl;
	}

	cout << "\n\n\n";
}


void Client::startPathDesign()
{
	clearCurrentPath();

	currentPathMutex.lock();
	for (int i = 0; i < DEFAULT_PATH_LENGTH; i++) {
		unsigned int randomIndex = Utility::generateRandomNumber(0, this->receivedRelays.size() - 1);
		string currentIp = receivedRelays.at(randomIndex).getIp();
		unsigned short currentPort = receivedRelays.at(randomIndex).getPort();

		RelayObject* currentNodeData = new RelayObject(currentIp, currentPort);

		currentPath.push_back(currentNodeData);
	}
	currentPathMutex.unlock();

	printPath();
}

void Client::handshakeWithCurrentPath()
{
	currentPathMutex.lock();
	for (int i = 0; i < currentPath.size(); i++) {
		RelayObject* currentNodeData = currentPath.at(i);
		currentPathMutex.unlock();

		handshakeWithNode(currentNodeData->getIp(), currentNodeData->getPort(), i);

		currentPathMutex.lock();
	}

	currentPathMutex.unlock();

	// Start timer
	std::thread connectionThread(&Client::checkConnectionAliveTimer, this);
	connectionThread.detach();
}

void Client::handshakeWithNode(string ip, unsigned short nodePort, unsigned int nodeIndex)
{
	ClientConnection nodeConnection(ip, nodePort, logger, &(this->eccHandler));
	nodeConnection.handshake();
	logger.success("Handshake with node " + to_string(nodePort) + " completed");

	string conversationIdEncrypted = nodeConnection.receiveData();

	string conversationIdDecrypted = nodeConnection.getAesHandler()->decrypt(conversationIdEncrypted);
	logger.success("Received conversationId: " + conversationIdDecrypted);

	currentPathMutex.lock();

	RelayObject* currentRelay = currentPath.at(nodeIndex);
	currentRelay->setAesKeys(nodeConnection.getAesHandler()->getAesKey());
	currentRelay->setEccHandler(nodeConnection.getParentECCHandler());
	currentRelay->setConversationId(conversationIdDecrypted);

	// Hey node, your next node is...
	if (nodeIndex < currentPath.size() - 1) {
		RelayObject* nextNode = currentPath.at(nodeIndex + 1);

		string nextNodeIp = nextNode->getIp();
		string nextNodePort = to_string(nextNode->getPort());

		nodeConnection.sendEncrypted(nextNodeIp + SPLITER + nextNodePort);
	}
	else {
		nodeConnection.sendEncrypted(Constants::EXIT_NODE_STRING);
	}

	currentPathMutex.unlock();

	nodeConnection.closeConnection();
}

bool Client::isPathDesignCommand(string command)
{
	// Check if the command starts with "START PATH DESIGN"
	if (command.find(START_PATH_DESIGN_STRING) == 0) {
		// Extract the substring after "START PATH DESIGN:"
		string subCommand = command.substr(START_PATH_DESIGN_STRING.size() + 1); // Length of "START PATH DESIGN:"
		// Check if the substring contains only digits
		if (std::all_of(subCommand.begin(), subCommand.end(), ::isdigit)) {
			return true;
		}
	}
	return false;
}

int Client::extractPathLength(string command)
{
	if (isPathDesignCommand(command)) {
		// Extract the substring after "START PATH DESIGN:"
		string subCommand = command.substr(START_PATH_DESIGN_STRING.size() + 1); // Length of "START PATH DESIGN:"
		// Convert the substring to an integer
		return std::stoi(subCommand);
	}
	return -1;
}

string Client::pathToString()
{
	// Return (ip:port) -> (ip:port) -> (ip:port) ... -> Destination

	string path = "";

	currentPathMutex.lock();

	for (RelayObject* tempNodeData : currentPath) {
		path += tempNodeData->toString() + " -> ";
	}

	currentPathMutex.unlock();

	return path + "Destination";
}

void Client::checkConnectionAliveTimer()
{
	while (!stop) {
		Sleep(1000);
		currentPathAliveTime += 1000;

		if (currentPathAliveTime > Constants::PATH_TIMEOUT) {
			logger.error("Path is alive for too long. Disconnecting...");

			clearCurrentPath();

			break;
		}
	}
}

ClientConnection* Client::connectToEntryNode()
{
	currentPathMutex.lock();
	RelayObject* entryNode = currentPath.at(0);
	currentPathMutex.unlock();
	cout << "Connecting to entry node whice is at port: " << entryNode->getPort() << endl;

	ClientConnection* entryNodeConnection = new ClientConnection(entryNode->getIp(), entryNode->getPort(), logger); // Connection made

	entryNodeConnection->receiveKeys(true);

	return entryNodeConnection;
}

void Client::sendData(string ip, unsigned short port, string message, ClientConnection* entryNodeConnection)
{

	if (!pathAvailable()) {
		throw std::runtime_error("Path not established yet");
	}

	string encryptedData = encrypt(ip, port, message);


	entryNodeConnection->sendData(encryptedData);
}

void Client::sendData(string username, string message, ClientConnection* entryNodeConnection)
{
	if (!pathAvailable()) {
		throw std::runtime_error("Path not established yet");
	}

	string encryptedData = encrypt(username, message);


	entryNodeConnection->sendData(encryptedData);
}

void Client::sendData(DestinationData dd, ClientConnection* entryNodeConnection)
{
	if (!pathAvailable()) {
		throw std::runtime_error("Path not established yet");
	}

	sendData(dd.getDestinationIP(), dd.getDestinationPort(), dd.getData(), entryNodeConnection);
}

void Client::sendToElectron(SOCKET socket, const string& message)
{
	IConnection::sendData(socket, message);
}

void Client::receiveInLoopToElectron(SOCKET electronSocket, ClientConnection** entryNodeConnection)
{
	while (!stop) {
		if (*entryNodeConnection == nullptr) { continue; }
		string data = (*entryNodeConnection)->receiveData();

		if (data.empty()) {
			continue;
		}

		string decrypted = decrypt(data);
		cout << "Received from next node: " << decrypted << endl;
		sendToElectron(electronSocket, decrypted);
	}
}

string Client::encrypt(string ip, unsigned short port, string message)
{

	if (!pathAvailable()) {
		throw std::runtime_error("Path not established yet");
	}

	if (ip.size() < 7 || ip.size() > 15 || port == 0) {
		throw std::runtime_error("IP and port must be provided");
	}

	string encrypted = ip + SPLITER + to_string(port) + SPLITER + message;

	RelayObject* relayObject;


	currentPathMutex.lock();
	for (int i = currentPath.size() - 1; i >= 0; i--) {
		relayObject = currentPath.at(i);
		AesKey* currentKeys = relayObject->getAesKeys();

		encrypted = AesHandler::encryptAES(encrypted, currentKeys);
		encrypted = relayObject->getConversationIdEncrypted() + encrypted;
	}

	currentPathMutex.unlock();

	return encrypted;
}

string Client::encrypt(string username, string message)
{
	if (!pathAvailable()) {
		throw std::runtime_error("Path not established yet");
	}

	if (username.empty()) {
		throw std::runtime_error("Username must be provided");
	}

	string encrypted = username + SPLITER + message;

	RelayObject* relayObject;

	currentPathMutex.lock();
	for (int i = currentPath.size() - 1; i >= 0; i--) {
		relayObject = currentPath.at(i);
		AesKey* currentKeys = relayObject->getAesKeys();

		encrypted = AesHandler::encryptAES(encrypted, currentKeys);
		encrypted = relayObject->getConversationIdEncrypted() + encrypted;
	}

	currentPathMutex.unlock();

	return encrypted;
}

string Client::decrypt(string encrypted)
{

	if (!pathAvailable()) {
		throw std::runtime_error("Path not established yet");
	}

	string decrypted = encrypted;
	RelayObject* relayObject;

	currentPathMutex.lock();
	for (int i = 0; i < currentPath.size(); i++) {
		relayObject = currentPath.at(i);
		AesKey* currentKeys = relayObject->getAesKeys();

		decrypted = AesHandler::decryptAES(decrypted, currentKeys);
	}

	currentPathMutex.unlock();

	return decrypted;
}

void Client::printNodes()
{
	currentPathMutex.lock();

	for (RelayObject* relay : currentPath) {
		cout << "Node: " << relay->getPort() << " - " << relay->getConversationId() << "\n";
	}

	currentPathMutex.unlock();

	cout << "\n\n\n";
}

unsigned long long Client::getConnectionTime()
{
	return currentPathAliveTime;
}

bool Client::isErrorResponse(std::string& responseString)
{
	try {
		return JsonResponse(responseString).isErrorResponse();
	}
	catch (std::exception& e) {
		return false;
	}
}

void Client::clearCurrentPath()
{
	currentPathMutex.lock();

	for (int i = 0; i < currentPath.size(); i++) {
		delete currentPath.at(i);
	}

	currentPath.clear();

	currentPathMutex.unlock();
}

void Client::printPath()
{
	cout << "Path: [";

	for (RelayObject* tempNodeData : currentPath) {
		cout << tempNodeData->getIp() << ":" << tempNodeData->getPort() << " -> ";
	}

	cout << "Destination]" << endl;
}

bool Client::pathAvailable()
{
	currentPathMutex.lock();

	bool available = currentPath.size() >= DEFAULT_PATH_LENGTH && !pathIsTooOld();

	currentPathMutex.unlock();

	return available;
}

bool Client::pathIsTooOld()
{
	return currentPathAliveTime > Constants::PATH_TIMEOUT - 10000; // Margin of 10 seconds to prevent errors
}


