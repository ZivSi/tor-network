#include "Node.h"

// Port variable
unsigned short Node::PORT = SERVER_PORT + 1;

Node::Node() : logger("Node " + to_string(Node::PORT)), stop(false), myIP(getLocalIpv4()), IConnection(myIP, Node::PORT, &logger)
{
	myPort = Node::PORT;
	Node::PORT += 1;

	this->parentConnection = new ClientConnection(SERVER_IP, SERVER_PORT, logger);

	thread acceptInThread(&Node::acceptSocket, this, getSocket());
	acceptInThread.detach();
}

Node::Node(string parentIP, unsigned short parentPort) : logger("Node " + to_string(Node::PORT)), stop(false), myIP(getLocalIpv4()), IConnection(myIP, Node::PORT, &logger)
{
	myPort = Node::PORT;
	Node::PORT += 1;

	this->parentConnection = new ClientConnection(parentIP, parentPort, logger);

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
		logger.clientEvent("Client is performing an handshake");

		clientHandshake(clientSocket);
		return;
	}

	logger.success("Node (?) connected");

	try {
		string conversationId = Utility::extractConversationId(received);
		string decryptedConversationId = getECCHandler()->decrypt(conversationId);

		ConversationObject* currentConversation = findConversationBy(decryptedConversationId);
		currentConversation->setPrvNodeSOCKET(clientSocket);

		if (!conversationExists(currentConversation)) {
			logger.error("Did not find conversation with ID of " + conversationId);

			closesocket(clientSocket);
			return;
		}

		if (!currentConversation->isExitNode()) { handleNode(clientSocket, currentConversation, received); }
		else { handleNodeAsExit(clientSocket, currentConversation, received); }
	}
	catch (std::runtime_error e) {
		logger.error("Runtime error in handleNode");

		cout << e.what() << endl;
	}
	catch (std::exception e) {
		logger.error("Exception in handleNode");

		cout << e.what() << endl;
	}
}

bool Node::conversationExists(ConversationObject* currentConversation)
{
	return currentConversation != NULL;
}

void Node::listenToNextNode(SOCKET previousNodeSocket, ConversationObject* currentConversation)
{
	ClientConnection* nextNode = currentConversation->getNxtNode();


	while (!stop) {
		if (currentConversation->isTooOld()) {
			removeConversationFromMap(currentConversation->getConversationId());
			if (currentConversation != nullptr) { delete currentConversation; }

			closesocket(previousNodeSocket);
			return;
		}

		string received = nextNode->receiveData();

		if (dataLegit(received)) {
			cout << "Data legit in listenNextNode: " << received << endl;
			string encrypted = AesHandler::encryptAES(received, currentConversation->getKey());

			sendData(previousNodeSocket, encrypted);
		}
	}
}


void Node::handleNode(SOCKET previousNodeSocket, ConversationObject* currentConversation, string initialMessage)
{
	logger.success("Handling node\n");

	string received = initialMessage;
	ClientConnection* nextNode = currentConversation->getNxtNode();

	if (nextNode == nullptr) {
		nextNode = new ClientConnection(currentConversation->getNxtIP(), currentConversation->getNxtPort(), this->logger);
		nextNode->receiveKeys(true);

		currentConversation->setNxtNode(nextNode);
	}


	thread listenNextNodeThread(&Node::listenToNextNode, this, previousNodeSocket, currentConversation);
	listenNextNodeThread.detach();

	while (!stop) {
		if (currentConversation->isTooOld()) {
			removeConversationFromMap(currentConversation->getConversationId());
			if (currentConversation != nullptr) { delete currentConversation; }
			closesocket(previousNodeSocket);
			return;
		}

		if (dataLegit(received)) {
			string decrypted = AesHandler::decryptAES(received.substr(UUID_ENCRYPTED_SIZE, received.size()), currentConversation->getKey());

			nextNode->sendData(decrypted);
		}

		received = receiveData(previousNodeSocket);
	}
}

void Node::listenToHosts(ConversationObject* currentConversation) {
	SOCKET previousNodeSocket = currentConversation->getPrvNodeSOCKET();

	while (!stop) {
		currentConversation->collectMessages();

		if (currentConversation->isTooOld()) {
			cout << "Conversation is too old" << endl;
			// handleNodeAsExit() will remove the conversation from the map
			return;
		}

		while (!currentConversation->isQueueEmpty()) {
			string encryptedReversedMessage = currentConversation->getFirstMessage();

			sendData(previousNodeSocket, encryptedReversedMessage);
		}

		// Sleep(200); // TODO: Remove for high performance mode
	}
}

void Node::handleNodeAsExit(SOCKET previousNodeSocket, ConversationObject* currentConversation, string initialMessage)
{
	logger.success("Handling node as exit\n");

	thread listenNextNodeThread(&Node::listenToHosts, this, currentConversation);
	listenNextNodeThread.detach();

	string received = initialMessage;

	while (!stop) {
		if (currentConversation->isTooOld()) {
			removeConversationFromMap(currentConversation->getConversationId());
			delete currentConversation;
			closesocket(previousNodeSocket);
			return;
		}


		string decrypted = AesHandler::decryptAES(received.substr(UUID_ENCRYPTED_SIZE, received.size()), currentConversation->getKey());
		DestinationData dd(decrypted);

		ConnectionPair host(dd.getDestinationIP(), dd.getDestinationPort());


		try {
			if (!currentConversation->isDestinationActive(dd)) {
				cout << "Connection to " << dd.getDestinationIP() << ":" << dd.getDestinationPort() << " is not active\n";
				currentConversation->addActiveConnection(dd);
				cout << "Connected to " << dd.getDestinationIP() << ":" << dd.getDestinationPort() << endl;
			}

			currentConversation->getActiveConnection(dd)->sendData(dd.getData());
		}
		catch (std::runtime_error e) {
			string errorMessage = "Node {" + this->getIP() + ":" + to_string(this->getPort()) + "} - ID: " + currentConversation->getConversationId() + SPLITER + "Can't connect to host at " + host.toString();

			logger.error(errorMessage);

			sendErrorToClient(previousNodeSocket, errorMessage, currentConversation->getKey());
		}

		received = receiveData(previousNodeSocket);
	}
}

ConversationObject* Node::findConversationBy(string conversationId) {
	auto it = conversationsMap.find(conversationId);

	if (it != conversationsMap.end()) {
		return it->second;
	}

	return NULL;
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

		logger.log("Removed conversation from map: " + conversationId);

		return;
	}

	logger.error("Couldn't find conversation in map to remove. ID: " + conversationId + "\n");
}

string Node::getLocalIpv4() {
	char hostbuffer[256];
	struct addrinfo hints, * res, * p;
	int status;
	char ipstr[INET6_ADDRSTRLEN];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // Use AF_INET to force IPv4
	hints.ai_socktype = SOCK_STREAM;

	// Retrieve hostname
	if (gethostname(hostbuffer, sizeof(hostbuffer)) == -1) {
		throw std::runtime_error("Error getting hostname.");
	}

	// Retrieve host information
	if ((status = getaddrinfo(hostbuffer, NULL, &hints, &res)) != 0) {
		throw std::runtime_error("Error on status getaddrinfo");
	}

	// Loop through all the results and get the first IPv4 address
	for (p = res; p != NULL; p = p->ai_next) {
		if (p->ai_family == AF_INET) { // IPv4
			struct sockaddr_in* ipv4 = (struct sockaddr_in*)p->ai_addr;
			// Convert the IP to a string and return it
			inet_ntop(AF_INET, &(ipv4->sin_addr), ipstr, sizeof ipstr);
			freeaddrinfo(res); // Free memory allocated by getaddrinfo
			return std::string(ipstr);
		}
	}

	freeaddrinfo(res); // Free memory allocated by getaddrinfo
	throw std::runtime_error("Error getting local IPv4.");
}

bool Node::dataLegit(string& data)
{
	return !data.empty();
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
		logger.error("Error in decryptECC (receivedAESKeys)");

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
		string nextNodeProperties = receiveData(clientSocket);
		decryptedNextNodeProperties = AesHandler::decryptAES(nextNodeProperties, &aesPair);

		cout << "Received next node properties: " << decryptedNextNodeProperties << endl;

		try {
			if (decryptedNextNodeProperties == Constants::EXIT_NODE_STRING) {
				throw std::runtime_error("Received destination. I am the exit node");
			}

			vector<string> parts = Utility::splitString(decryptedNextNodeProperties, SPLITER);

			string nextNodeIP = parts[0];
			unsigned short nextNodePortInt = static_cast<unsigned short>(stoi(parts[1]));

			if (!(Utility::isValidIpv4(nextNodeIP) && Utility::isValidPort(nextNodePortInt))) {
				throw std::runtime_error("Invalid IP or port");
			}

			logger.clientEvent("Received next node properties: " + nextNodeIP + ":" + to_string(nextNodePortInt));

			currentConversation->setNxtIP(nextNodeIP);
			currentConversation->setNxtPort(nextNodePortInt);
		}
		catch (std::runtime_error e) {
			cout << e.what() << endl;

			// We received destination. We are the exit node
			currentConversation->setAsExitNode();

			logger.log("Received destination. I am the exit node\n");
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

void Node::sendErrorToClient(SOCKET previousNodeSocket, string errorMessage, AesKey* aesKey)
{
	string encryptedErrorMessage = AesHandler::encryptAES(errorMessage, aesKey);

	sendData(previousNodeSocket, encryptedErrorMessage);
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
			logger.error("Error in sendAlive");
			cout << e.what() << endl;
		}

		Sleep(MAX_TIME_ALIVE / 3);
	}
}

string Node::receiveECCKeys(SOCKET clientSocket)
{
	return receiveKeys(clientSocket);
}