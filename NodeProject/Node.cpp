#include "Node.h"

// Port variable
unsigned short Node::PORT = SERVER_PORT + 1;

Node::Node() : logger("Node [" + to_string(Node::PORT) + "]"), stop(false), IConnection("127.0.0.1", Node::PORT, &logger)
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
}


void Node::acceptSocket(SOCKET socket)
{
	// Accept a client socket
	sockaddr_in client;
	int clientSize = sizeof(client);

	logger.log("Waiting for connections...");

	SOCKET clientSocket = accept(socket, (sockaddr*)&client, &clientSize);

	if (clientSocket == INVALID_SOCKET) {
		cerr << "Accept failed with error: " << WSAGetLastError() << endl;
		exit(1);
	}

	cout << "Node [" << myPort << "] accepted connection from client" << endl;

	// Handle the client in a new thread
	thread clientThread(&Node::handleClient, this, clientSocket);
	clientThread.detach();
}


void Node::handleClient(SOCKET clientSocket)
{
	sendECCKey(clientSocket);
	cout << "\b\b to the client" << endl;

	string received = this->receiveECCKeys(clientSocket); // eccEncrypted(ConversationId) + aesEncrypted(Data) or handshake (begins with 0Y0*åH╬)

	if (isHandshake(received)) {
		logger.clientEvent("Client wants to perform handshake");

		clientHandshake(clientSocket);
		return;
	}


	string conversationId = Utility::extractConversationId(received);
	string decryptedConversationId = eccHandler.decrypt(conversationId);

	ConversationObject* currentConversation = findConversationBy(decryptedConversationId);

	if (currentConversation->isEmpty()) {
		delete currentConversation;

		closesocket(clientSocket);

		return;
	}

	string receivedDecrypted = AesHandler::decryptAES(received.substr(UUID_LEN), currentConversation->getKey());

	ClientConnection* nextNode = currentConversation->getNxtNode();
	unsigned short nextNodePort = currentConversation->getNxtPort();

	if (!isConnectedTo(nextNode)) {
		ClientConnection* nextNodeConnection = new ClientConnection("127.0.0.1", nextNodePort, logger);
		currentConversation->setNxtNode(nextNodeConnection);
	}


	sendData(currentConversation->getNxtNode()->getSocket(), receivedDecrypted);
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
		return;
	}


	string extractedAes = "";
	string extractedIv = "";

	Utility::extractAESKey(decryptedAESKeys, extractedAes);
	Utility::extractAESIv(decryptedAESKeys, extractedIv);

	SecByteBlock aesKey = AesKey::StringToSecByteBlock(extractedAes);
	SecByteBlock aesIv = AesKey::StringToSecByteBlock(extractedIv);

	logger.keysInfo("Extracted AES key and IV from received data");

	// ----------- Build conversation object and send conversation id to client ------------
	string conversationId = ConversationObject::generateID();

	AesKey aesPair(aesKey, aesIv);
	ConversationObject* currentConversation = new ConversationObject(conversationId, aesPair);

	std::pair<string, ConversationObject*> newPair(conversationId, currentConversation);

	conversationsMap.insert(newPair);

	string encryptedId = AesHandler::encryptAES(conversationId, &aesPair, true);

	sendData(clientSocket, encryptedId);

	// Receive next node port
	string nextNodePort = receiveData(clientSocket);
	string decryptedNextNodePort = AesHandler::decryptAES(nextNodePort, &aesPair);
	try {
		unsigned short nextNodePortInt = stoi(decryptedNextNodePort);
		currentConversation->setNxtPort(nextNodePortInt);
	}
	catch (Exception) {
		logger.error("Error in stoi(decryptedNextNodePort)");

		closesocket(clientSocket);


		removeConversationFromMap(conversationId);
		delete currentConversation;

		return;
	}

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

	string keysStr = aesHandler.getAesKey().serializeKey();

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
