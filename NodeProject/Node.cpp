#include "Node.h"

// Port variable
unsigned short Node::PORT = SERVER_PORT + 1;

Node::Node() : logger("Node " + to_string(Node::PORT)), stop(false), IConnection("127.0.0.1", Node::PORT, &logger)
{
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
}


void Node::acceptSocket(SOCKET socket)
{
	logger.log("Waiting for connections...");

	while (true) {
		sockaddr_in client;
		int clientSize = sizeof(client);

		SOCKET clientSocket = accept(socket, (sockaddr*)&client, &clientSize);

		if (clientSocket == INVALID_SOCKET) {
			cerr << "Accept failed with error: " << WSAGetLastError() << endl;
			WSACleanup();
			exit(1);
		}

		logger.clientEvent("accepted connection from client");

		// Handle the client in a new thread
		thread clientThread(&Node::handleClient, this, clientSocket);
		clientThread.detach();
	}
}


void Node::handleClient(SOCKET clientSocket)
{
	sendECCKey(clientSocket);

	string received = "";
	try {
		received = receiveBlock(clientSocket);
	}
	catch (std::runtime_error e) {

		logger.error("Error in receiving ECC keys (or data from previous node)");

		return;
	}

	const string receivedConst = received;

	if (isHandshake(receivedConst)) {
		logger.clientEvent("Client wants to perform handshake");

		clientHandshake(clientSocket);
		return;
	}

	logger.log("Node connected");

	// --------------- Determine the conversation and set the prvNode ---------------

	string conversationId = Utility::extractConversationId(receivedConst);
	string decryptedConversationId = getECCHandler()->decrypt(conversationId);

	ConversationObject* currentConversation = findConversationBy(decryptedConversationId);

	if (!conversationExists(currentConversation)) {
		delete currentConversation;

		closesocket(clientSocket);

		return;
	}

	currentConversation->setPrvNode(clientSocket);

	// --------------- We handle the first packet here before handling the rest of the packets in the node ---------------
	string dataPart = receivedConst.substr(UUID_ENCRYPTED_SIZE, receivedConst.size());

	string decryptedData = AesHandler::decryptAES(dataPart, currentConversation->getKey());


	// If we are the exit node, we send the data to the destination
	if (currentConversation->isExitNode()) {
		string ip = Utility::extractIpAddress(decryptedData);
		unsigned short port = Utility::extractPort(decryptedData);

		logger.log("Received destination. Sending data to " + ip + ":" + to_string(port));

		decryptedData = decryptedData.substr(ip.size() + SPLITER.size() + to_string(port).size() + SPLITER.size(), decryptedData.size());

		ClientConnection destinationConnection(ip, port, logger);

		try {
			destinationConnection.connectInLoop();
		}
		catch (Exception) {
			logger.error("Error in connecting to destination");
			return;
		}

		destinationConnection.sendData(decryptedData);

		destinationConnection.closeConnection();
	}

	// Is next node (/ dest) active?
	else {
		if (!currentConversation->getNxtNode()->isConversationActive()) {
			try {
				currentConversation->getNxtNode()->connectInLoop();
			}
			catch (Exception) {
				logger.error("Error in connecting to next node");
				return;
			}
		}

		currentConversation->getNxtNode()->sendData(decryptedData);
	}


	// --------------- Handle the rest of the packets in the handler ---------------
	handleNode(clientSocket, currentConversation);
}

bool Node::conversationExists(ConversationObject* currentConversation)
{
	return !currentConversation->isEmpty();
}

void Node::handleNode(SOCKET nodeSocket, ConversationObject* conversation)
{
	string received;

	while (true) {
		try {
			received = receiveData(nodeSocket);
		}
		catch (Exception) { // Lets hope the node will be able to recover and reconnect
			logger.error("Error in receiving data from node. Closing socket...");

			closesocket(nodeSocket);
			return;
		}

		if (received.empty()) {
			logger.error("Received empty data from node. Closing socket...");
			closesocket(nodeSocket);
			return;
		}

		string decrypted = AesHandler::decryptAES(received, conversation->getKey());

		if (decrypted.empty()) {
			logger.error("Received empty data from node. Closing socket...");
			closesocket(nodeSocket);
			return;
		}

		try {
			conversation->getNxtNode()->sendData(decrypted);

		}
		catch (Exception) {

			try {
				conversation->getNxtNode()->connectInLoop();
			}
			catch (...) {
				logger.error("Error in connecting to next node");
				return;
			}
		}
	}
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
	string nextNodePort;
	string decryptedNextNodePort;

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
		nextNodePort = receiveData(clientSocket);
		decryptedNextNodePort = AesHandler::decryptAES(nextNodePort, &aesPair);

		try {
			unsigned short nextNodePortInt = stoi(decryptedNextNodePort);
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

	string formattedData = to_string(this->myPort) + SPLITER + "Public Key!" + SPLITER + to_string(currentTime) + SPLITER + to_string(nonPrime) + SPLITER + to_string(randomNumber) + SPLITER + to_string(modulusBase);

	formattedData = formattedData + SPLITER + Utility::hashStr(formattedData + PEPPER);
	formattedData = formattedData + SPLITER + Utility::hashStr(formattedData + PEPPER2);

	return formattedData;
}


void Node::sendAlive()
{
	while (true) {
		Sleep(MAX_TIME_ALIVE / 3);

		try {
			parentConnection->connectInLoop();

			this->handshake(parentConnection);

			parentConnection->closeConnection();
		}
		catch (Exception) {
			logger.error("Error in sendAlive (probably handshake)");
		}
	}
}

string Node::receiveECCKeys(SOCKET clientSocket)
{
	return receiveKeys(clientSocket);
}

string Node::receiveBlock(SOCKET clientSocket)
{
	int maxTries = 40;
	string received = "";

	do {
		received = this->receiveData(clientSocket);
	} while (received.empty() && maxTries--);

	if (received.empty()) {
		throw std::runtime_error("Error in receiving data");
	}

	return received;
}
