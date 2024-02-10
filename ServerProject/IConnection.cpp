#include "IConnection.h"

IConnection::IConnection(string ip, unsigned short port, Logger* logger) {
	this->ip = ip;
	this->port = port;
	this->logger = logger;


	connection = initWSASocket();
	bindSocket(connection);
	listenSocket(connection);
	acceptSocket(connection);
}

IConnection::~IConnection()
{
	closeConnection();
}

SOCKET IConnection::initWSASocket() {
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

void IConnection::bindSocket(SOCKET socket) {
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(port);
	hint.sin_addr.S_un.S_addr = INADDR_ANY;

	bind(socket, (sockaddr*)&hint, sizeof(hint));
}

void IConnection::listenSocket(SOCKET socket) {
	if (socket == INVALID_SOCKET) {
		logger->error("Can't create a socket, Err #" + WSAGetLastError());
		exit(1);
	}

	int listening = listen(socket, SOMAXCONN);
	if (listening == SOCKET_ERROR) {
		logger->error("Can't listen on socket, Err #" + WSAGetLastError());
		exit(1);
	}
}

void IConnection::acceptSocket(SOCKET socket) {
	// Meant to be overrided
}

void IConnection::handleClient(SOCKET clientSocket) {
	// Meant to be overrided
}

void IConnection::sendData(SOCKET connection, const string& data) {
	try {
		// First send size of data
		size_t dataSize = data.size();

		send(connection, reinterpret_cast<const char*>(&dataSize), sizeof(size_t), 0);

		send(connection, data.data(), dataSize, 0);
	}
	catch (...) {
		logger->error("Couldn't send data. Client disconnected");

		throw std::runtime_error("Couldn't send data. Client disconnected");
	}
}

void IConnection::sendKeys(SOCKET connection, const string& keyStr) {
	sendData(connection, keyStr);
}

void IConnection::sendECCKey(SOCKET connection)
{
	eccHandlerMutex.lock();
	string keysStr = eccHandler.serializeKey();
	eccHandlerMutex.unlock();

	sendKeys(connection, keysStr);

	logger->keysInfo("Sent public key");
}

string IConnection::receiveData(SOCKET connection) {
	size_t dataSize = 0;

	unsigned long long received = recv(connection, reinterpret_cast<char*>(&dataSize), sizeof(size_t), 0);
	// First, receive the size of the data
	if (received < 0) {
		logger->error("Data size is: " + to_string(dataSize));
		throw std::runtime_error("Failed to receive data size");
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
			throw std::runtime_error("Failed to receive data");
		}
		else if (bytesReceived == 0) {
			// Connection closed prematurely
			throw std::runtime_error("Connection closed prematurely");
		}

		// Append received data to the string
		data.append(buffer.data(), bytesReceived);
		totalReceived += bytesReceived;
	}

	return data;
}


string IConnection::receiveKeys(SOCKET connection) {
	return receiveData(connection);
}

void IConnection::closeConnection() {
	WSACleanup();
	closesocket(connection);
}

SOCKET IConnection::getSocket()
{
	return connection;
}

unsigned short IConnection::getPort()
{
	return port;
}

string IConnection::getIP()
{
	return ip;
}

ECCHandler* IConnection::getECCHandler()
{
	return &eccHandler;
}

string IConnection::encryptECC(string data)
{
	eccHandlerMutex.lock();
	try {
		string encryptedData = this->eccHandler.encrypt(data);
		eccHandlerMutex.unlock();

		return encryptedData;
	}
	catch (...) {
		eccHandlerMutex.unlock();
		throw;
	}
}

string IConnection::decryptECC(string data)
{
	eccHandlerMutex.lock();
	try {
		string decryptedData = this->eccHandler.decrypt(data);
		eccHandlerMutex.unlock();

		return decryptedData;
	}
	catch (...) {
		eccHandlerMutex.unlock();
		cout << "Error during decryption: " << data << endl;
		throw "Error during decryption";
	}
}
