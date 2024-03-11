#include "Client.h"

Client::Client() : logger("Client"), clientConnection("127.0.0.1", SERVER_PORT, logger), stop(false)
{
	clientConnection.handshake();
	clientConnection.sendEncrypted("Hi. I'm a client"); // Initial message

	waitForNodes();

	receiveResponseFromServer();

	startPathDesign();

	handshakeWithCurrentPath();
}

Client::~Client() {
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

string Client::encrypt(string ip, unsigned short port, string data)
{

	if (!pathAvailable()) {
		throw std::runtime_error("Path not established yet");
	}

	string encrypted = ip + SPLITER + to_string(port) + SPLITER + data;
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

	bool available = currentPath.size() == Constants::DEFAULT_PATH_LENGTH && !pathIsTooOld();

	currentPathMutex.unlock();

	return available;
}

bool Client::pathIsTooOld()
{
	return currentPathAliveTime > Constants::PATH_TIMEOUT;
}


