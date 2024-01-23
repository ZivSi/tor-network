#include "Node.h"

// Port variable
unsigned short Node::PORT = SERVER_PORT + 1;

Node::Node() : logger("Node [" + to_string(Node::PORT) + "]"), stop(false)
{
	myPort = Node::PORT;
	Node::PORT += 1;

	this->parentConnection = new ClientConnection("127.0.0.1", SERVER_PORT, logger);
}

Node::~Node()
{
}


SOCKET Node::initWSASocket() {
	WSADATA wsaData;
	WORD version = MAKEWORD(2, 2);
	int wsaResult = WSAStartup(version, &wsaData);

	if (wsaResult != 0) {
		cerr << "Can't start Winsock, Err #" << wsaResult << endl;
		exit(1);
	}

	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
	if (listening == INVALID_SOCKET) {
		cerr << "Can't create a socket, Err #" << WSAGetLastError() << endl;
		exit(1);
	}

	return listening;
}

void Node::bindSocket(SOCKET socket)
{
	// Bind the socket
	sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(this->myPort);
	server.sin_addr.s_addr = INADDR_ANY;

	int iResult = bind(socket, (sockaddr*)&server, sizeof(server));
	if (iResult == SOCKET_ERROR) {
		cerr << "Bind failed with error: " << WSAGetLastError() << endl;
		exit(1);
	}
}

void Node::listenSocket(SOCKET socket)
{
	// Listen on the socket
	int iResult = listen(socket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		cerr << "Listen failed with error: " << WSAGetLastError() << endl;
		exit(1);
	}
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

	// Handle the client
	this->handleClient(clientSocket);
}

void Node::sendData(string data, SOCKET connection) {
	size_t dataSize = data.size();

	send(connection, reinterpret_cast<const char*>(&dataSize), sizeof(size_t), 0);

	send(connection, data.data(), dataSize, 0);
}

string Node::receiveData(SOCKET clientSocket)
{
	size_t dataSize = 0;

	// First, receive the size of the data
	recv(clientSocket, reinterpret_cast<char*>(&dataSize), sizeof(size_t), 0);

	// Allocate a buffer to store the received data
	char* buffer = new char[dataSize];

	// Receive the key
	recv(clientSocket, buffer, dataSize, 0);

	// Create a string from the received data
	string data(buffer, dataSize);

	delete[] buffer;


	return data;

}

void Node::handleClient(SOCKET clientSocket)
{
	string received = receiveData(clientSocket); // ConversationId/0Data

	if (isHandshake(received)) {
		logger.clientEvent("Client wants to perform handshake");

		clientHandshake(clientSocket);
		return;
	}

	/*
	// Node sending data
	string conversationId = received.until("\0");
	string conversationIdDecrypted = eccHandler.decrypt(conversationId);


	ConversationObject* currentConversation;
	// Extract appropriate aes key by the conversation id
	for (ConversationObject& tempConversation : conversations) {
		if (tempConversation.getConversationId() == conversationIdDecrypted) {
			currentConversation = &tempConversation;
			return;
		}
	}

	AesKey currentKey = currentConversation->getKey();
	SOCKET nextNode = currentConversation->getNxtNode();

	received = received.from('\0');
	string receivedDecrypted = AesHandler::decryptAES(received, currentKey.getKey(), currentKey.getIv());

	if (nextNode not in aliveSockets) // We are already connected to the node
	{
		try {
			connectToNode();
		}
		catch () {
			sendErrorToClient(currentConversation);

			deleteConversation(currentConversation);

			return;
		}
	}

	sendToNode(nextNode, receivedDecrypted);
	*/
}

bool Node::isHandshake(string received)
{
	// If client handshake, the beggining will be ECC key
	string typicalBeggining = "0Y0*åH╬";

	string keyPart = received.substr(0, typicalBeggining.size());

	return keyPart == typicalBeggining;
}

void Node::clientHandshake(SOCKET clientSocket)
{
	/*
-	receiveECCKeys() // Done
-	sendECCKeys() // Done
-	receive AES key
-	decrypt the rest using the client’s key
-	generate conversation id and AES key
-	send the AES key and conversation id
-	create conversation object
-	close connection

	*/

	sendECCKeys(clientSocket);

	string receivedAES;
	try {
		receivedAES = this->receiveAESKey(clientSocket);
		logger.keysInfo("Received AES keys from client");
	}
	catch (Exception) {
		logger.error("Error in receiving AES keys");
		return;
	}

	string aesKeys;
	try {
		aesKeys = eccHandler.decrypt(receivedAES);
	}
	catch (Exception e) {
		logger.error("Error in eccHandler.decrypt");
		cout << e.what() << endl;
		return;
	}


	string extractedAes = "";
	string extractedIv = "";

	Utility::extractAESKey(aesKeys, extractedAes);
	Utility::extractAESIv(aesKeys, extractedIv);

	SecByteBlock aesKey = AesHandler::StringToSecByteBlock(extractedAes);
	SecByteBlock aesIv = AesHandler::StringToSecByteBlock(extractedIv);

	logger.keysInfo("Extracted AES key and IV from received data");

	// Build conversation object
	string conversationId = ConversationObject::generateID();
	AesKey aesPair(&extractedAes, &extractedIv);
	ConversationObject currentConversation(conversationId, aesPair);

	conversations.push_back(currentConversation);

	string encryptedId = AesHandler::encryptAES(conversationId, aesPair.getKey(), aesPair.getIv());

	sendData(encryptedId, clientSocket);

	closesocket(clientSocket);
}

void Node::start() {
	Node::PORT++;
	this->myPort = Node::PORT;

	// Ecc keys already generated in the constructor

	logger.log("Starting Node: " + to_string(this->myPort));

	this->runMyServer(this->myPort);

	thread t(&Node::sendAlive, this);
	t.join();
}

void Node::runMyServer(unsigned short port)
{
	this->myServerSocket = this->initWSASocket();
	logger.log("Initialized WSA");

	// Bind the socket
	this->bindSocket(this->myServerSocket);

	// Listen on the socket
	this->listenSocket(this->myServerSocket);

	thread t(&Node::acceptSocket, this, this->myServerSocket);
	t.detach();
}

ClientConnection* Node::connectToParent(string parentIp, unsigned short parentPort, bool repeat = true)
{
	parentConnection->closeConnection();
	this->parentConnection = new ClientConnection("127.0.0.1", SERVER_PORT, logger);

	return this->parentConnection;
}

void Node::handshake(ClientConnection* parentConnection)
{
	// the data should be in the format of:
	// Port:publickey:CurrentTime:serializedKey:randomNonPrimeNumber:number that make(random % (number – port) == 1:hashOfText + PEPPER : hashOfText + PEPPER2

	string receivedECCKeys = parentConnection->receiveKeys();
	logger.keysInfo("Received server's ECC key");

	// Send ECC
	parentConnection->sendKeys(eccHandler.serializeKey());

	sendAESKeys(parentConnection, receivedECCKeys);

	unsigned long int nonPrime = Utility::generateNonPrime();
	unsigned long int modulusBase = Utility::findModuloBase(nonPrime, myPort);
	unsigned long int randomNumber = Utility::generateRandomNumber(0, 71067106);
	unsigned long long currentTime = Utility::capture_time();

	string formattedData = to_string(this->myPort) + SPLITER + "Public Key!" + SPLITER + to_string(currentTime) + SPLITER + to_string(nonPrime) + SPLITER + to_string(randomNumber) + SPLITER + to_string(modulusBase);

	formattedData = formattedData + SPLITER + Utility::hashStr(formattedData + PEPPER);
	formattedData = formattedData + SPLITER + Utility::hashStr(formattedData + PEPPER2);

	string encryptedData = aesHandler.encrypt(formattedData);

	parentConnection->sendData(encryptedData);
}

void Node::sendAlive()
{
	while (true) {
		Sleep(MAX_TIME_ALIVE / 3);

		try {
			parentConnection = this->connectToParent("127.0.0.1", SERVER_PORT);

			this->handshake(parentConnection);

			parentConnection->closeConnection();
		}
		catch (Exception) {
			logger.error("Error in sendAlive (probably handshake)");
		}
	}
}

void Node::sendAESKeys(ClientConnection* parentConnection, string receivedECCKeys)
{
	cout << "Server's public key is: " << receivedECCKeys << endl;
	ECCHandler serverECCHandler(receivedECCKeys);

	string keysStr = aesHandler.formatKeyForSending(aesHandler.getKey());
	keysStr += aesHandler.formatKeyForSending(aesHandler.getIv());

	string encryptedKeys = serverECCHandler.encrypt(keysStr);

	parentConnection->sendData(encryptedKeys);

	logger.keysInfo("Sent symmetric key (AES)");
}


void Node::sendECCKeys(SOCKET clientSocket)
{
	string keysStr = eccHandler.serializeKey();
	size_t dataSize = keysStr.size();

	send(clientSocket, reinterpret_cast<const char*>(&dataSize), sizeof(size_t), 0);

	send(clientSocket, keysStr.data(), dataSize, 0);

	logger.keysInfo("Sent public key (ECC)");
}


string Node::receiveKeys(SOCKET clientSocket)
{
	int dataSize = 0;

	// First, receive the size of the data
	recv(clientSocket, reinterpret_cast<char*>(&dataSize), sizeof(size_t), 0);

	// Allocate a buffer to store the received data
	char* buffer = new char[dataSize];

	// Receive the key
	recv(clientSocket, buffer, dataSize, 0);

	// Create a string from the received data
	string keysStr(buffer, dataSize);

	delete[] buffer;


	return keysStr;
}

string Node::receiveECCKeys(SOCKET clientSocket)
{
	return receiveKeys(clientSocket);
}

string Node::receiveAESKey(SOCKET clientSocket) {
	return receiveKeys(clientSocket);
}
