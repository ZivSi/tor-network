#include "Client.h"

Client::Client() : logger("Client"), clientConnection("127.0.0.1", SERVER_PORT, logger)
{
	clientConnection.handshake();
	clientConnection.sendEncrypted("Hi. I'm a client"); // Initial message
}

Client::~Client() {
	clientConnection.closeConnection();

	for (int i = 0; i < currentPath.size(); i++) {
		delete currentPath.at(i);
	}

	currentPath.clear();

	receivedPorts.clear();

	logger.log("Client destroyed");
}

void Client::waitForNodes()
{
	receiveResponseFromServer();

	while (receivedPorts.empty()) {
		clientConnection.closeConnection();

		Sleep(3000);

		clientConnection.connectInLoop();
		clientConnection.handshake();
		clientConnection.sendEncrypted("Hi. I'm a client");

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

	receivedPorts.clear();

	// Deserialize the received data
	size_t numPorts = received.size() / sizeof(unsigned short);
	receivedPorts.resize(numPorts);

	std::memcpy(receivedPorts.data(), received.data(), received.size());

	cout << "Received ports: [";
	for (unsigned short port : receivedPorts) {
		cout << port << ", ";
	}

	// Delete last comma
	cout << "\b\b]" << endl;
}

void Client::startPathDesign()
{
	clearCurrentPath();

	for (int i = 0; i < DEFAULT_PATH_LENGTH; i++) {
		unsigned int randomIndex = Utility::generateRandomNumber(0, this->receivedPorts.size() - 1);
		unsigned short currentPort = receivedPorts.at(randomIndex);

		RelayObject* currentNodeData = new RelayObject(currentPort);

		currentPath.push_back(currentNodeData);
	}

	printPath();
}

void Client::handshakeWithCurrentPath()
{
	for (int i = 0; i < currentPath.size(); i++) {
		RelayObject* currentNodeData = currentPath.at(i);

		handshakeWithNode(currentNodeData->getPort(), i);
	}
}

void Client::handshakeWithNode(unsigned short nodePort, unsigned int nodeIndex)
{
	ClientConnection nodeConnection("127.0.0.1", nodePort, logger, &(this->eccHandler));
	nodeConnection.handshake();
	logger.success("Handshake with node " + to_string(nodePort) + " completed");

	string conversationIdEncrypted = nodeConnection.receiveData();

	string conversationIdDecrypted = nodeConnection.getAesHandler()->decrypt(conversationIdEncrypted);
	logger.success("Received conversationId: " + conversationIdDecrypted);

	RelayObject* currentRelay = currentPath.at(nodeIndex);
	currentRelay->setAesKeys(nodeConnection.getAesHandler()->getAesKey());
	currentRelay->setEccHandler(nodeConnection.getParentECCHandler());
	currentRelay->setConversationId(conversationIdDecrypted);

	// Hey node, your next node is...
	if (nodeIndex < currentPath.size() - 1) {
		RelayObject* nextNode = currentPath.at(nodeIndex + 1);
		string nextNodePort = to_string(nextNode->getPort());

		nodeConnection.sendEncrypted(nextNodePort);
	}
	else {
		nodeConnection.sendEncrypted("Destination");
	}

	nodeConnection.closeConnection();
}

ClientConnection* Client::connectToEntryNode()
{
	RelayObject* entryNode = currentPath.at(0);
	cout << "Connecting to entry node whice is at port: " << entryNode->getPort() << endl;

	ClientConnection* entryNodeConnection = new ClientConnection("127.0.0.1", entryNode->getPort(), logger); // Connection made

	return entryNodeConnection;
}

void Client::sendData(string ip, unsigned short port, string message, ClientConnection* entryNodeConnection)
{
	string encryptedData = encrypt(ip, port, message);

	cout << "Encrypted message:\n" << encryptedData << endl;

	entryNodeConnection->sendData(encryptedData);
}

string Client::encrypt(string ip, unsigned short port, string data)
{
	string encrypted = ip + SPLITER + to_string(port) + SPLITER + data;
	RelayObject* relayObject;

	// encrypted = eccEncrypted(conversationId) + aesEncrypted(data)
	for (int i = currentPath.size() - 1; i >= 0; i--) {
		relayObject = currentPath.at(i);
		AesKey* currentKeys = relayObject->getAesKeys();

		encrypted = AesHandler::encryptAES(encrypted, currentKeys);
		encrypted = relayObject->getConversationIdEncrypted() + encrypted;
	}

	return encrypted;
}

void Client::printNodes()
{
	for (RelayObject* relay : currentPath) {
		cout << "Node: " << relay->getPort() << " - " << relay->getConversationId() << endl;
	}

	cout << "\n\n\n";
}

void Client::clearCurrentPath()
{
	for (int i = 0; i < currentPath.size(); i++) {
		delete currentPath.at(i);
	}

	currentPath.clear();
}

void Client::printPath()
{
	cout << "Path: [";

	for (RelayObject* tempNodeData : currentPath) {
		cout << tempNodeData->getPort() << " -> ";
	}

	cout << "Destination]" << endl;
}


