#include "ClientConnection.h"

ClientConnection::ClientConnection(string ip, unsigned short port) : logger("ClientConnection [" + ip + ":" + to_string(port) + "]")
{
	this->ip = ip;
	this->port = port;

	initWSASocket();
	connectInLoop();
}

ClientConnection::ClientConnection(string ip, unsigned short port, Logger logger) : logger(logger)
{
	this->ip = ip;
	this->port = port;

	initWSASocket();
	connectInLoop();
}

ClientConnection::ClientConnection(string ip, unsigned short port, Logger logger, ECCHandler* eccHandler) : logger(logger)
{
	this->ip = ip;
	this->port = port;
	this->eccHandler = *eccHandler;

	initWSASocket();
	connectInLoop();
}

ClientConnection::~ClientConnection()
{
	if (connectionActive) { closeConnection(); }

	WSACleanup();
}

void ClientConnection::initWSASocket()
{
	WSADATA wsaData;
	WORD version = MAKEWORD(2, 2);
	int wsOk = WSAStartup(version, &wsaData);
	if (wsOk != 0)
	{
		throw std::runtime_error("Can't initialize winsock! Quitting");
	}
}

SOCKET ClientConnection::connectToServer()
{
	SOCKET connection = socket(AF_INET, SOCK_STREAM, 0);
	if (connection == INVALID_SOCKET)
	{
		cout << "Can't create socket! Quitting" << endl;

		// std runtime error
		throw std::runtime_error("INVALID_SOCKET");
	}

	sockaddr_in hint{};
	hint.sin_family = AF_INET;
	hint.sin_port = htons(port);
	inet_pton(AF_INET, ip.c_str(), &hint.sin_addr);

	int connResult = connect(connection, (sockaddr*)&hint, sizeof(hint));
	if (connResult == SOCKET_ERROR)
	{
		cout << "Can't connect to server! Quitting" << endl;

		throw std::runtime_error("SOCKET_ERROR");
	}

	if (this->port != SERVER_PORT) {
		logger.log("Connected to " + this->ip + ":" + to_string(this->port));
	}

	connectionActive = true;

	return connection;
}

SOCKET ClientConnection::connectInLoop()
{
	while (failedAttempts < MAX_ATTEMPTS) {
		try {
			this->connection = connectToServer();
			this->failedAttempts = 0;

			break;
		}
		catch (std::runtime_error e) {
			failedAttempts++;

			logger.error("Couldn't connect to server");
			cout << "Runtime error: " << e.what() << endl;

			Sleep(DELAY_IN_CONNECTION_LOOP);
		}
	}

	if (failedAttempts == MAX_ATTEMPTS) {
		throw std::runtime_error("Reached maximum number of failed attempts to connect to server (" + to_string(MAX_ATTEMPTS) + ")");
	}

	return this->connection;
}

void ClientConnection::handshake()
{
	receiveKeys(true); // Initialize parent ECC
	sendECCKeys();
	sendAESKeys();
}


void ClientConnection::sendData(string data) {
	size_t dataSize = data.size();

	try {
		// Send the size of the data
		size_t bytesSent = send(connection, reinterpret_cast<const char*>(&dataSize), sizeof(size_t), 0);

		if (bytesSent == -1) {
			// If send() returns -1, handle the error
			throw std::runtime_error("Failed to send data size");
		}

		if (static_cast<size_t>(bytesSent) != sizeof(size_t)) {
			// Handle the case where not all bytes of the size were sent
			throw std::runtime_error("Incomplete data size sent");
		}

		// Send the data itself
		bytesSent = send(connection, data.data(), dataSize, 0);
		if (bytesSent == -1) {
			// If send() returns -1, handle the error
			throw std::runtime_error("Failed to send data");
		}

		if (static_cast<size_t>(bytesSent) != dataSize) {
			// Handle the case where not all bytes of the data were sent
			throw std::runtime_error("Incomplete data sent");
		}
	}
	catch (std::runtime_error e) {
		logger.error(e.what());
		closeConnection();

		throw e;
	}
}

void ClientConnection::sendKeys(string keysStr)
{
	sendData(keysStr);
}

void ClientConnection::sendEncrypted(string data)
{
	sendData(aesHandler.encrypt(data));
}

string ClientConnection::receiveDataFromTcp() 
{
	string data;

	constexpr size_t chunkSize = 4096;
	vector<char> buffer(chunkSize);

	size_t bytesReceived = recv(connection, buffer.data(), chunkSize, 0);

	data.append(buffer.data(), bytesReceived);

	return data;
}

string ClientConnection::receiveData() {
	size_t dataSize = 0;

	try {
		// First, receive the size of the data
		if (recv(connection, reinterpret_cast<char*>(&dataSize), sizeof(size_t), 0) < 0) {
			// throw std::runtime_error("Failed to receive data size");

			return "";
		}

		string data;
		data.reserve(dataSize); // Reserve space for the entire data

		// Receive the data in chunks
		constexpr size_t chunkSize = 4096;
		vector<char> buffer(chunkSize);

		size_t totalReceived = 0;
		while (totalReceived < dataSize) {
			size_t bytesToReceive = min(chunkSize, dataSize - totalReceived);
			size_t bytesReceived = recv(connection, buffer.data(), bytesToReceive, 0);

			if (bytesReceived < 0) {
				// Handle receive error
				// throw std::runtime_error("Failed to receive data");

				return "";
			}
			else if (bytesReceived == 0) {
				// Connection closed prematurely
				// throw std::runtime_error("Connection closed prematurely");

				return "";
			}

			// Append received data to the string
			data.append(buffer.data(), bytesReceived);
			totalReceived += bytesReceived;
		}

		return data;
	}
	catch (std::runtime_error& e) {
		// logger.error(e.what());
		// closeConnection();

		// throw e;
	}
}


string ClientConnection::receiveKeys(bool initialize = true)
{
	string receivedKeys = receiveData();

	if (initialize) { this->initializeParentECC(receivedKeys); }

	return receivedKeys;
}

void ClientConnection::initializeParentECC(string receivedECCKeys)
{
	this->parentECCHandler.initialize(receivedECCKeys);
}

void ClientConnection::sendECCKeys()
{

	string eccKeysSerialized = this->eccHandler.serializeKey();
	sendKeys(eccKeysSerialized);

	logger.keysInfo("Sent public key");
}

void ClientConnection::sendECCKeys(ECCHandler* eccHandler)
{

	string eccKeysSerialized = eccHandler->serializeKey();
	sendKeys(eccKeysSerialized);

	logger.keysInfo("Sent public key");
}

void ClientConnection::sendAESKeys()
{
	string keysStr = aesHandler.getAesKey().serializeKey();

	string encryptedKeys = parentECCHandler.encrypt(keysStr);

	sendData(encryptedKeys);

	// logger.keysInfo("Sent symmetric key (AES) with hex: " + Utility::asHex(keysStr));
}


SOCKET ClientConnection::getSocket() const
{
	return connection;
}

unsigned short ClientConnection::getPort() const
{
	return port;
}

string ClientConnection::getIP() const
{
	return ip;
}

AesKey ClientConnection::getAesKey() const
{
	return this->aesHandler.getAesKey();
}

ECCHandler* ClientConnection::getParentECCHandler()
{
	return &(this->parentECCHandler);
}

AesHandler* ClientConnection::getAesHandler()
{
	return &(this->aesHandler);
}

void ClientConnection::closeConnection()
{
	closesocket(connection);

	if (this->port != SERVER_PORT) {
		logger.log("Closed connection to " + this->ip + ":" + to_string(this->port));
	}

	connectionActive = false;
}

bool ClientConnection::isConnectionActive() const
{
	return connectionActive;
}

