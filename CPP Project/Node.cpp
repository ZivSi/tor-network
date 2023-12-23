#include "Node.h"

Node::Node()
{
	this->myPort = Constants::SERVER_PORT + 1;

	this->myServerSocket = initWSASocket();
	this->bindSocket(this->myServerSocket);
	this->listenSocket(this->myServerSocket);
	this->acceptInThread(this->myServerSocket);

	string aesKeyForSending = AesHandler::format_key_for_sending(this->aesObject.getKey());
	string aesIvForSending = AesHandler::format_key_for_sending(this->aesObject.getIv());

	this->aliveFormat = std::to_string(myPort) + Constants::SPLITER + aesKeyForSending + Constants::SPLITER + aesIvForSending;

	this->sendAliveMessage(this->myServerSocket);
}

Node::~Node() {
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

	cout << "Created a socket" << endl;

	return listening;
}

void Node::bindSocket(SOCKET socket) {
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(myPort);
	hint.sin_addr.S_un.S_addr = INADDR_ANY;

	if (bind(socket, (sockaddr*)&hint, sizeof(hint)) == SOCKET_ERROR) {
		cerr << "Can't bind to port: " << myPort << endl;

		myPort++;

		bindSocket(socket);

		return;
	}

	cout << "Binded socket to port " << myPort << endl;
}

void Node::listenSocket(SOCKET socket) {
	if (socket == INVALID_SOCKET) {
		cerr << "Can't listen on socket, Err #" << WSAGetLastError() << endl;
		exit(1);
	}

	int listening = listen(socket, SOMAXCONN);
	if (listening == SOCKET_ERROR) {
		cerr << "Can't listen on socket, Err #" << WSAGetLastError() << endl;
		exit(1);
	}

	cout << "Server is listening on port " << myPort << endl;
}

void Node::sendAliveMessage(SOCKET socket)
{
	while (true) {
		sendData(socket, aliveFormat);

		Sleep(MAX_TIME_ALIVE / 2);
	}
}

vector<string> Node::extractData(string decrypted)
{
	// decrypted = nodeServerPort + Constants::SPLITER + AesHandler::encryptAES(encrypted, nodeAesKey, nodeAesIv);

	vector<string> extracted = Utility::splitString(decrypted, Constants::SPLITER);

	return extracted;
}

void Node::sendData(SOCKET socket, string data) {
	send(socket, data.c_str(), data.size() + 1, 0);
}

void Node::acceptInThread(SOCKET socket) {
	std::thread acceptThread(&Node::acceptSocket, this, socket);
	acceptThread.detach();
}

void Node::acceptSocket(SOCKET socket) {
	try {
		while (true) {
			SOCKET clientSocket = accept(socket, NULL, NULL);

			if (clientSocket == INVALID_SOCKET) {
				cerr << "Can't accept client socket, Err #" << WSAGetLastError() << endl;
				exit(1);
			}

			string clientIP = getIPFrom(clientSocket);
			int clientPort = getPortFrom(clientSocket);

			cout << "Accepted client " << clientIP << ":" << clientPort << endl;

			// Receive data from the client
			string recievedData = receiveData(clientSocket);
			cout << "Received data: " << recievedData << endl;

			closesocket(clientSocket);

			string decrypted = aesObject.decrypt(recievedData);
			cout << "Decrypted data: " << decrypted << endl;

			vector<string> extracted = extractData(decrypted);

			int destinationPort = std::stoi(extracted[0]);
			string encrypted = extracted[1];

			cout << "Destination port: " << destinationPort << endl;
			cout << "Encrypted data: " << encrypted << endl;

			// Send the received data to the next node
			sendToNextNode(encrypted, destinationPort, encrypted);
		}
	}
	catch (const std::exception& e) {
		cerr << "Error in acceptSocket(): " << e.what() << endl;

	}
}

int Node::getPortFrom(SOCKET socket) {
	sockaddr_in clientAddr;
	int addrSize = sizeof(clientAddr);
	getpeername(socket, (struct sockaddr*)&clientAddr, &addrSize);

	return ntohs(clientAddr.sin_port);
}

string Node::getIPFrom(SOCKET socket) {
	sockaddr_in clientAddr;
	int addrSize = sizeof(clientAddr);
	getpeername(socket, (struct sockaddr*)&clientAddr, &addrSize);

	char ip[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &(clientAddr.sin_addr), ip, INET_ADDRSTRLEN);
	return string(ip);
}

string Node::receiveData(SOCKET socket) {
	char buf[RECEIVE_SIZE];
	ZeroMemory(buf, RECEIVE_SIZE);

	int bytesReceived = recv(socket, buf, RECEIVE_SIZE, 0);

	if (bytesReceived == SOCKET_ERROR) {
		cerr << "Error in recv(). Quitting" << endl;
		exit(1);
	}

	return string(buf, 0, bytesReceived);
}

SOCKET Node::connectToServer(string serverIP, int serverPort) {
	SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (clientSocket == INVALID_SOCKET) {
		cerr << "Can't create client socket, Err #" << WSAGetLastError() << endl;
		exit(1);
	}

	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(serverPort);
	inet_pton(AF_INET, serverIP.c_str(), &(serverAddr.sin_addr));

	int connectionResult = connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	if (connectionResult == SOCKET_ERROR) {
		cerr << "Can't connect to server, Err #" << WSAGetLastError() << endl;
		closesocket(clientSocket);
		exit(1);
	}

	cout << "Connected to server " << serverIP << ":" << serverPort << endl;

	return clientSocket;
}

void Node::sendToNextNode(string encrypted, int port, string data) {
	// Just connect, send, and disconnect

	SOCKET nextNodeSocket = connectToServer("127.0.0.1", port);

	sendData(nextNodeSocket, encrypted);

	closesocket(nextNodeSocket);

	cout << "Sent to next node" << endl;
}

