#include "ClientConnection.h"

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
	closeConnection();
	WSACleanup();
}

void ClientConnection::initWSASocket()
{
	WSADATA wsaData;
	WORD version = MAKEWORD(2, 2);
	int wsOk = WSAStartup(version, &wsaData);
	if (wsOk != 0)
	{
		throw string("Can't initialize winsock! Quitting");
	}
}

SOCKET ClientConnection::connectToServer()
{
	SOCKET connection = socket(AF_INET, SOCK_STREAM, 0);
	if (connection == INVALID_SOCKET)
	{
		cout << "Can't create socket! Quitting" << endl;

		throw "Can't create socket! Quitting";
	}

	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(port);
	inet_pton(AF_INET, ip.c_str(), &hint.sin_addr);

	int connResult = connect(connection, (sockaddr*)&hint, sizeof(hint));
	if (connResult == SOCKET_ERROR)
	{
		cout << "Can't connect to server! Quitting" << endl;

		throw "Can't connect to server! Quitting";
	}

	logger.log("Connected to " + this->ip + ":" + to_string(this->port));

	return connection;
}

SOCKET ClientConnection::connectInLoop()
{
	while (failedAttempts < 20) {
		try {
			this->connection = connectToServer();

			break;
		}
		catch (...) {
			failedAttempts++;
			logger.error("Couldn't connect to server");

			Sleep(3000);
		}
	}

	return this->connection;
}

void ClientConnection::handshake()
{
	receiveKeys(true); // Initialize parent ECC
	sendECCKeys();
	sendAESKeys();
}


void ClientConnection::sendData(string data)
{
	size_t dataSize = data.size();

	// First send the size of the data
	send(connection, reinterpret_cast<const char*>(&dataSize), sizeof(size_t), 0);

	// Send the data
	send(connection, data.data(), dataSize, 0);
}

void ClientConnection::sendKeys(string keysStr)
{
	sendData(keysStr);
}

void ClientConnection::sendEncrypted(string data)
{
	sendData(aesHandler.encrypt(data));
}

string ClientConnection::receiveData() {
	size_t dataSize = 0;

	// First, receive the size of the data
	if (recv(connection, reinterpret_cast<char*>(&dataSize), sizeof(size_t), 0) < 0) {
		// Handle receive error
		throw std::runtime_error("Failed to receive data size");
	}

	// Allocate a buffer to store the received data
	vector<char> buffer(dataSize);

	// Receive the data
	size_t totalReceived = 0;
	while (totalReceived < dataSize) {
		size_t bytesReceived = recv(connection, buffer.data() + totalReceived, dataSize - totalReceived, 0);

		if (bytesReceived < 0) {
			// Handle receive error
			throw std::runtime_error("Failed to receive data");
		}
		else if (bytesReceived == 0) {
			// Connection closed prematurely
			throw std::runtime_error("Connection closed prematurely");
		}
		totalReceived += bytesReceived;
	}

	// Create a string from the received data
	string data(buffer.begin(), buffer.end());

	return data;
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

	logger.keysInfo("Sent symmetric key (AES) with hex: " + Utility::asHex(keysStr));

	if (this->port == SERVER_PORT) {
		return;
	}

	cout << "Sent AES keys: " << encryptedKeys << endl;
	cout << "AES keys decrypted: " << keysStr << endl;
}


SOCKET ClientConnection::getSocket()
{
	return connection;
}

unsigned short ClientConnection::getPort()
{
	return port;
}

string ClientConnection::getIP()
{
	return ip;
}

AesKey ClientConnection::getAesKey()
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

	logger.log("Closed connection to " + this->ip + ":" + to_string(this->port));
}

