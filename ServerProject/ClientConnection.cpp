#include "ClientConnection.h"

ClientConnection::ClientConnection(string ip, unsigned short port, Logger logger) : logger(logger)
{
	this->ip = ip;
	this->port = port;

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
		throw string("Can't create socket! Quitting");
	}

	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(port);
	inet_pton(AF_INET, ip.c_str(), &hint.sin_addr);

	int connResult = connect(connection, (sockaddr*)&hint, sizeof(hint));
	if (connResult == SOCKET_ERROR)
	{
		cout << "Can't connect to server! Quitting" << endl;
		throw string("Can't connect to server! Quitting");
	}

	logger.log("Connected to " + this->ip + ":" + to_string(this->port));

	return connection;
}

SOCKET ClientConnection::connectInLoop()
{
	closeConnection();

	while (true) {
		try {
			this->connection = connectToServer();

			break;
		}
		catch (exception e) {
			logger.error("Couldn't connect to server");

			Sleep(3000);
		}
	}

	return this->connection;
}

void ClientConnection::handshake()
{
	receiveKeys(true);
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

string ClientConnection::receiveData()
{
	size_t dataSize = 0;

	// First, receive the size of the data
	recv(connection, reinterpret_cast<char*>(&dataSize), sizeof(size_t), 0);

	// Allocate a buffer to store the received data
	char* buffer = new char[dataSize];

	// Receive the data
	recv(connection, buffer, dataSize, 0);

	// Create a string from the received data
	string data(buffer, dataSize);

	delete[] buffer;

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

void ClientConnection::sendAESKeys()
{
	string keysStr = aesHandler.serializeKey();

	string encryptedKeys = parentECCHandler.encrypt(keysStr);

	sendData(encryptedKeys);

	logger.keysInfo("Sent symmetric key (AES)");
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

void ClientConnection::closeConnection()
{
	closesocket(connection);
}

