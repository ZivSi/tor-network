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

	startPathDesign();

	for (int i = 0; i < currentPath.size(); i++) {
		RelayObject* currentNodeData = currentPath.at(i);

		handshakeWithNode(currentNodeData->getPort(), i);
	}

	cout << "Handshake completed" << endl;
}

void Client::receiveResponseFromServerInLoop()
{
	while (true) {
		receiveResponseFromServer();

		clientConnection.closeConnection();

		Sleep(3000);

		clientConnection.connectInLoop();
		clientConnection.handshake();
		clientConnection.sendEncrypted("Hi. I'm a client");
	}
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

void Client::handshakeWithNode(unsigned short nodePort, unsigned int nodeIndex)
{
	ClientConnection nodeConnection("127.0.0.1", nodePort, logger, &(this->eccHandler));
	nodeConnection.handshake();
	logger.success("Handshake with node " + to_string(nodePort) + " completed");

	string conversationIdEncrypted = nodeConnection.receiveData();

	string conversationIdDecrypted = nodeConnection.getAesHandler()->decrypt(conversationIdEncrypted, true);
	logger.success("Received conversationId: " + conversationIdDecrypted);

	RelayObject* currentRelay = new RelayObject(nodePort, nodeConnection.getAesKey(), nodeConnection.getParentECCHandler(), conversationIdDecrypted);

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

void Client::sendData(string ip, unsigned short port, string message)
{
	string encryptedData = encrypt(message);

	cout << "Encrypted message:\n" << encryptedData << endl;
}

string Client::encrypt(string data)
{
	string encrypted = data;

	// encrypted = eccEncrypted(conversationId) + aesEncrypted(data)
	for (int i = currentPath.size() - 1; i >= 0; i--) {
		RelayObject* relayObject = currentPath.at(i);
		AesKey* currentKeys = relayObject->getAesKeys();

		encrypted = AesHandler::encryptAES(encrypted, currentKeys);
		encrypted = relayObject->getConversationIdEncrypted() + encrypted;
	}

	return encrypted;
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


