#include "Node.h"

// Port variable
unsigned short Node::PORT = SERVER_PORT + 1;

Node::Node() : logger("Node " + to_string(Node::PORT)), stop(false), IConnection("127.0.0.1", Node::PORT, &logger)
{
	//  Set by the local ip of the current machine (such as 10.0.0.10)
	myIP = getLocalIp();
	myPort = Node::PORT;
	Node::PORT += 1;

	this->parentConnection = new ClientConnection("127.0.0.1", SERVER_PORT, logger);

	thread acceptInThread(&Node::acceptSocket, this, getSocket());
	acceptInThread.detach();
}

Node::~Node()
{
	closeConnection();

	for (auto it = conversationsMap.begin(); it != conversationsMap.end(); ++it) {
		delete it->second;
	}

	conversationsMap.clear();

	delete parentConnection;

	logger.log("Node destroyed");

	stop = true;
}


void Node::acceptSocket(SOCKET socket) {
	sockaddr_in client{};
	int clientSize = sizeof(client);
	SOCKET clientSocket;

	logger.log("Waiting for connections...");

	try {
		while (!stop) {
			clientSocket = accept(socket, (sockaddr*)&client, &clientSize);

			if (clientSocket == INVALID_SOCKET) {
				int error = WSAGetLastError();
				if (error != WSAEWOULDBLOCK) {
					logger.error("Can't accept client socket, Error: " + std::to_string(error));
				}

				continue;
			}

			logger.clientEvent("Accepted connection from client");

			std::thread clientThread(&Node::handleClient, this, clientSocket);
			clientThread.detach();
		}
	}
	catch (const std::exception& e) {
		cerr << "Error in acceptSocket(): " << e.what() << endl;
	}
	catch (...) {
		cerr << "Unknown error in acceptSocket()" << endl;
	}
}


void Node::handleClient(SOCKET clientSocket)
{
	sendECCKey(clientSocket);

	string received = "";
	try {
		received = this->receiveData(clientSocket); // ECC keys or data from previous node
	}
	catch (std::runtime_error e) {

		logger.error("Error in receiving ECC keys (or data from previous node)");

		return;
	}


	if (isHandshake(received)) {
		logger.clientEvent("Client wants to perform handshake");

		clientHandshake(clientSocket);
		return;
	}

	logger.log("Node connected");

	try {
		handleNode(clientSocket, received);
	}
	catch (std::runtime_error e) {
		logger.error("Error in handleNode");

		cout << e.what() << endl;
	}
	catch (std::exception e) {
		logger.error("Error in handleNode");

		cout << e.what() << endl;
	}
}

bool Node::conversationExists(ConversationObject* currentConversation)
{
	return !currentConversation->isEmpty();
}

void Node::handleNode(SOCKET nodeSocket, string initialMessage)
{
	ConversationObject* currentConversation = nullptr;
	string received = initialMessage;
	string decryptedConversationId;

	do {

		if (currentConversation != nullptr && currentConversation->isTooOld()) {
			logger.log("Conversation is too old. Removing from map");

			removeConversationFromMap(decryptedConversationId);
			delete currentConversation;

			closesocket(nodeSocket);

			return;
		}

		if (received.empty()) {
			continue;
		}


		string conversationId = Utility::extractConversationId(received);
		decryptedConversationId = getECCHandler()->decrypt(conversationId);

		currentConversation = findConversationBy(decryptedConversationId);

		if (!conversationExists(currentConversation)) {
			delete currentConversation;

			closesocket(nodeSocket);

			return;
		}


		currentConversation->setPrvNode(nodeSocket);

		string dataPart = received.substr(UUID_ENCRYPTED_SIZE, received.size());

		string decryptedData = AesHandler::decryptAES(dataPart, currentConversation->getKey());

		if (currentConversation->isExitNode()) {
			// extract data, ip, port
			// check if conversation is open in the map
			// if exists, send the packet. if not, create new clientconnection, connect, send data and add to the map
			logger.success("Received data from exit node: " + decryptedData);

			// Extract data
			vector<string> parts = Utility::splitString(decryptedData, SPLITER);

			string ip = parts[0];
			unsigned short port = static_cast<unsigned short>(stoi(parts[1]));

			// TODO: Check if conversation is open in the map
			// For now just create new client connection and send data
			ClientConnection* destination = new ClientConnection(ip, port, logger);

			destination->sendData(parts[2]);
			destination->closeConnection();

			return;
		}

		if (currentConversation->getNxtNode() == nullptr) {
			ClientConnection* nextNodeConnection = new ClientConnection("127.0.0.1", currentConversation->getNxtPort(), this->logger);
			currentConversation->setNxtNode(nextNodeConnection);
		}

		if (!currentConversation->getNxtNode()->isConversationActive()) {
			currentConversation->getNxtNode()->connectInLoop();
		}

		currentConversation->getNxtNode()->sendData(decryptedData);

		received = this->receiveData(nodeSocket);

	} while (!stop);
}


ConversationObject* Node::findConversationBy(string conversationId) {
	auto it = conversationsMap.find(conversationId);

	if (it != conversationsMap.end()) {
		return it->second;
	}

	return new ConversationObject();
}

bool Node::isConnectedTo(ClientConnection* nextNode)
{
	return nextNode != nullptr;
}

void Node::removeConversationFromMap(string conversationId)
{
	auto it = conversationsMap.find(conversationId);

	if (it != conversationsMap.end()) {
		delete it->second;
		conversationsMap.erase(it);
	}

	logger.log("Removed conversation from map: " + conversationId);
}

bool Node::isHandshake(string received)
{
	// If client handshake, the beggining will be ECC key. Another node won't send ECC key
	string typicalBeggining = "0"; // TODO: Change to ECC key

	string keyPart = received.substr(0, typicalBeggining.size());

	return keyPart == typicalBeggining;
}

void Node::clientHandshake(SOCKET clientSocket)

{
	string receivedAESKeys;

	try {
		receivedAESKeys = receiveKeys(clientSocket);
		logger.keysInfo("Received AES keys from client");

	}
	catch (Exception) {
		logger.error("Error in receiving AES keys");
		return;
	}


	string decryptedAESKeys;
	try {
		decryptedAESKeys = decryptECC(receivedAESKeys);

		logger.keysInfo("Decrypted AES keys from client: " + Utility::asHex(decryptedAESKeys));

	}
	catch (...) {
		logger.error("Error in decryptECC(receivedAESKeys)");

		return;
	}

	ConversationObject* currentConversation = nullptr;
	string conversationId;
	AesKey aesPair;
	string encryptedId;
	string nextNodeProperties;
	string decryptedNextNodeProperties;

	try {
		aesPair = AesKey::decryptedAESKeysToPair(decryptedAESKeys);

		// ----------- Build conversation object and send conversation id to client ------------
		conversationId = ConversationObject::generateID();

		currentConversation = new ConversationObject(conversationId, aesPair);

		std::pair<string, ConversationObject*> newPair(conversationId, currentConversation);

		conversationsMap.insert(newPair);

		encryptedId = AesHandler::encryptAES(conversationId, &aesPair);

		sendData(clientSocket, encryptedId);

		// Receive next node port
		// TODO: add ip
		string nextNodeProperties = receiveData(clientSocket);
		decryptedNextNodeProperties = AesHandler::decryptAES(nextNodeProperties, &aesPair);

		try {
			// TODO: split and only then try to convert to int and ip
			string nextNodeIP = decryptedNextNodeProperties.substr(0, Constants::IP_SIZE);
			unsigned short nextNodePortInt = stoi(decryptedNextNodeProperties.substr(Constants::IP_SIZE, Constants::PORT_SIZE));

			currentConversation->setNxtIP(nextNodeIP);
			currentConversation->setNxtPort(nextNodePortInt);
		}
		catch (...) {
			// We received destination. We are the exit node
			currentConversation->setAsExitNode();

			logger.log("Received destination. I am the exit node");
		}
	}

	catch (Exception) {
		logger.error("Error in clientHandshake");

		removeConversationFromMap(conversationId);

		if (currentConversation != nullptr) {
			delete currentConversation;
		}
	}

	logger.success("Handhsake with client successful. Closing socket...");
	closesocket(clientSocket);
}

void Node::start() {
	thread t(&Node::sendAlive, this);
	t.join();
}


void Node::handshake(ClientConnection* parentConnection)
{
	// the data should be in the format of:
	// Port:publickey:CurrentTime:serializedKey:randomNonPrimeNumber:number that make(random % (number – port) == 1:hashOfText + PEPPER : hashOfText + PEPPER2

	parentConnection->handshake();

	string formattedData = buildAliveFormat();

	parentConnection->sendEncrypted(formattedData);
}

void Node::sendAESKeys(ClientConnection* parentConnection, string receivedECCKeys)
{
	cout << "Server's public key is: " << receivedECCKeys << endl;
	ECCHandler serverECCHandler(receivedECCKeys);

	string keysStr = parentConnection->getAesKey().serializeKey();

	string encryptedKeys = serverECCHandler.encrypt(keysStr);

	parentConnection->sendData(encryptedKeys);

	logger.keysInfo("Sent symmetric key (AES)");
}

string Node::buildAliveFormat() {
	unsigned long int nonPrime = Utility::generateNonPrime();
	unsigned long int modulusBase = Utility::findModuloBase(nonPrime, myPort);
	unsigned long int randomNumber = Utility::generateRandomNumber(0, 71067106);
	unsigned long long currentTime = Utility::capture_time();

	string formattedData = myIP + SPLITER + to_string(this->myPort) + SPLITER + "Public Key!" + SPLITER + to_string(currentTime) + SPLITER + to_string(nonPrime) + SPLITER + to_string(randomNumber) + SPLITER + to_string(modulusBase);

	formattedData = formattedData + SPLITER + Utility::hashStr(formattedData + PEPPER);
	formattedData = formattedData + SPLITER + Utility::hashStr(formattedData + PEPPER2);

	return formattedData;
}


void Node::sendAlive()
{
	while (true) {
		try {
			parentConnection->connectInLoop();

			this->handshake(parentConnection);

			parentConnection->closeConnection();
		}
		catch (Exception e) {
			logger.error("Error in sendAlive (probably handshake)");
			cout << e.what() << endl;
		}

		Sleep(MAX_TIME_ALIVE / 3);
	}
}

string Node::receiveECCKeys(SOCKET clientSocket)
{
	return receiveKeys(clientSocket);
}


string Node::getLocalIp() {
	/*
	char hostbuffer[256];
	char* IPbuffer;
	struct hostent* host_entry;
	int hostname;

	// Retrieve hostname
	hostname = gethostname(hostbuffer, sizeof(hostbuffer));
	if (hostname == -1) {
		throw std::runtime_error("Error getting hostname.");
	}

	// Retrieve host information
	host_entry = gethostbyname(hostbuffer);
	if (host_entry == nullptr) {
		throw std::runtime_error("Error getting host information.");
	}

	// Convert the network address to a string
	IPbuffer = inet_ntoa(*((struct in_addr*)host_entry->h_addr_list[0]));

	return std::string(IPbuffer);
	*/

	return LOCALHOST;
}