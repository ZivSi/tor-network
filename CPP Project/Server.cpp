#include "Server.h"

Server::Server() {
	this->serverSocket = this->initWSASocket();
	this->bindSocket(this->serverSocket);
	this->listenSocket(this->serverSocket);
	this->acceptSocket(this->serverSocket);

	this->RSA_KEYS_FORMAT = to_string(rsaObject.getPublicKey()) + Constants::SPLITER + to_string(rsaObject.getModulus());
}

Server::~Server() {
	this->stop = true;
}

SOCKET Server::initWSASocket() {
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

void Server::bindSocket(SOCKET socket) {
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(Constants::SERVER_PORT);
	hint.sin_addr.S_un.S_addr = INADDR_ANY;

	bind(socket, (sockaddr*)&hint, sizeof(hint));

	cout << "Server is bound to port " << Constants::SERVER_PORT << endl;
}

void Server::listenSocket(SOCKET socket) {
	if (socket == INVALID_SOCKET) {
		cerr << "Can't listen on socket, Err #" << WSAGetLastError() << endl;
		exit(1);
	}

	int listening = listen(socket, SOMAXCONN);
	if (listening == SOCKET_ERROR) {
		cerr << "Can't listen on socket, Err #" << WSAGetLastError() << endl;
		exit(1);
	}

	cout << "Server is listening on port " << Constants::SERVER_PORT << endl;
}

string Server::receiveData(SOCKET socket) {
	char buf[RECEIVE_SIZE];
	ZeroMemory(buf, RECEIVE_SIZE);

	int bytesReceived = recv(socket, buf, RECEIVE_SIZE, 0);

	if (bytesReceived == SOCKET_ERROR) {
		cerr << "Error in recv(). Quitting" << endl;
		exit(1);
	}

	string received = string(buf, static_cast<size_t>(bytesReceived));

	cout << "Received: " << received;

	return received;
}

string Server::receiveDataWithTimeout(SOCKET socket, int timeoutInSeconds) {
	DWORD timeoutValue = timeoutInSeconds * 1000;

	if (setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeoutValue, sizeof(timeoutValue)) == SOCKET_ERROR) {
		std::cerr << "Error in setsockopt(). Quitting" << std::endl;
		exit(1);
	}

	char buf[RECEIVE_SIZE];
	ZeroMemory(buf, RECEIVE_SIZE);

	int bytesReceived = recv(socket, buf, RECEIVE_SIZE, 0);

	if (bytesReceived == SOCKET_ERROR) {
		std::cerr << "Error in recv(). Quitting" << std::endl;
		exit(1);
	}

	return std::string(buf, static_cast<size_t>(bytesReceived));
}

void Server::acceptSocket(SOCKET socket) {
	while (!stop) {
		SOCKET clientSocket = accept(socket, NULL, NULL);

		cout << "Accepted client" << endl;

		sendRSAKeys(clientSocket);

		cout << "Sent RSA keys" << endl;

		if (clientSocket == INVALID_SOCKET) {
			cerr << "Can't accept client socket, Err #" << WSAGetLastError() << endl;
			exit(1);
		}

		string clientIP = getIPFrom(clientSocket);
		int clientPort = getPortFrom(clientSocket);

		string response = receiveData(clientSocket);
		vector<int64_t> evaluated = Utility::eval_list(response);
		string decryptedResponse = rsaObject.decrypt(evaluated);

		bool isNode = determine(decryptedResponse);

		if (isNode) {
			NodeData node = extractNodeData(decryptedResponse);

			node.set_last_seen(Utility::capture_time());

			nodes.push_back(node);

			thread nodeThread(&Server::handleNode, this, &node);
			nodeThread.detach();
		}
		else {
			ClientData client = extractClientData(decryptedResponse);

			client.set_last_seen(Utility::capture_time());

			clients.push_back(client);

			thread clientThread(&Server::handleClient, this, client);
			clientThread.detach();

		}
	}
}

bool Server::determine(string decryptedResponse) {
	int have = Utility::splitString(decryptedResponse, Constants::SPLITER).size();
	int need = Utility::splitString(Constants::NODE_INIT_FORMAT, Constants::SPLITER).size();

	return have == need;
}

void Server::sendData(SOCKET socket, string data) {
	send(socket, data.c_str(), data.size() + 1, 0);
}

void Server::sendRSAKeys(SOCKET socket) {
	sendData(socket, this->RSA_KEYS_FORMAT);
}

int Server::getPortFrom(SOCKET socket) {
	sockaddr_in clientAddr;
	int addrSize = sizeof(clientAddr);
	getpeername(socket, (struct sockaddr*)&clientAddr, &addrSize);

	return ntohs(clientAddr.sin_port);
}

string Server::getIPFrom(SOCKET socket) {
	sockaddr_in clientAddr;
	int addrSize = sizeof(clientAddr);
	getpeername(socket, (struct sockaddr*)&clientAddr, &addrSize);

	char ip[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &(clientAddr.sin_addr), ip, INET_ADDRSTRLEN);
	return string(ip);
}


NodeData Server::extractNodeData(string decryptedResponse) {
	vector<string> data = Utility::splitString(decryptedResponse, Constants::SPLITER);

	int nodeServerPort = stoi(data[0]);
	string nodePublicKey = data[1];
	string aes_key = data[2];
	string aes_iv = data[3];

	SecByteBlock aes_key_formatted = AesHandler::reformat_key_for_receiving(aes_key);
	SecByteBlock aes_iv_formatted = AesHandler::reformat_key_for_receiving(aes_iv);

	NodeData node(to_string(nodeServerPort), 0, nodePublicKey, aes_key_formatted, aes_iv_formatted);

	return node;
}


ClientData Server::extractClientData(string decryptedResponse) {
	vector<string> data = Utility::splitString(decryptedResponse, Constants::SPLITER);

	string aes_key = data[0];
	string aes_iv = data[1];

	SecByteBlock aes_key_formatted = AesHandler::reformat_key_for_receiving(aes_key);
	SecByteBlock aes_iv_formatted = AesHandler::reformat_key_for_receiving(aes_iv);

	// last seen, bytes sent, bytes received, aes key, aes iv
	ClientData client(0, 0, 0, aes_key_formatted, aes_iv_formatted);

	return client;
}

void Server::handleClient(ClientData client) {
	// All we do is to wait from message
	// When getting, splitting dest and data and start encryption and sending

	while (!stop) {
		string received = receiveData(client.get_socket());
		string decrypted = AesHandler::decryptAES(received, client.get_aes_key(), client.get_aes_iv());

		vector<string> splitted = Utility::splitString(decrypted, SPLITER);
		string dest = splitted.at(0);
		string data = splitted.at(1);

		unsigned long long time = Utility::capture_time();

		DataObject dataObject(data, client, time, 0, DEFAULT_PATH_LENGTH);

		vector<NodeData> nodes = Utility::generateRamdomPath(&(this->nodes));

		string encrypted = encrypt(dataObject.toString(), nodes);

		// Send
		sendToFirstNode(encrypted);
	}
}

void Server::handleNode(NodeData* node) {
	// We make sure the node is alive and gets it's status every couple of seconds

	// Receive in loop, register last seen and get its stats (capacity, etc...)

	bool continueHandling = true;
	while (!continueHandling) {
		string received = receiveDataWithTimeout(node->get_socket(), 5);

		if (received == "") {
			cout << "Node " << node->get_server_port() << " is dead" << endl;
			break;
		}

		// Should send new aes key and iv and sent bytes and received bytes
		try {
			vector<string> splitted = Utility::splitString(received, Constants::SPLITER);
			SecByteBlock aes_key = AesHandler::reformat_key_for_receiving(splitted.at(0));
			SecByteBlock aes_iv = AesHandler::reformat_key_for_receiving(splitted.at(1));
			int64_t sent_bytes = stoll(splitted.at(2));
			int64_t received_bytes = stoll(splitted.at(3));

			node->set_aes_key(aes_key);
			node->set_aes_iv(aes_iv);
			node->set_sent_bytes(sent_bytes);
			node->set_received_bytes(received_bytes);

			node->set_last_seen(Utility::capture_time());
		}
		catch (std::exception e) {
			cout << "Error in response format from node " << node->get_server_port() << endl;
			continueHandling = false;
		}

		// continueHandling if still in the nodes vector or error in the response format
		continueHandling = continueHandling && nodeInVector(*node);
	}
}


string Server::encrypt(string data, vector<NodeData> nodes) {
	string encrypted = data;
	NodeData previousNode;

	// A -> B -> C -> Client
	// A.encrypt(B.encrypt(C.encrypt(data)))
	for (int i = nodes.size() - 1; i >= 0; i--) {
		NodeData node = nodes.at(i);

		string nodeServerPort = node.get_server_port();
		SecByteBlock nodeAesKey = node.get_aes_key();
		SecByteBlock nodeAesIv = node.get_aes_iv();

		encrypted = nodeServerPort + Constants::SPLITER + AesHandler::encryptAES(encrypted, nodeAesKey, nodeAesIv);
	}

	// Remove first node's port (we don't need it since we are the one's who send it to him)
	encrypted = encrypted.substr(encrypted.find(Constants::SPLITER) + Constants::SPLITER.length());

	return encrypted;
}

void Server::sendToFirstNode(string data)
{
	// Conenct to the server of the first node and send the data
	int serverPort = stoi(nodes.at(0).get_server_port());
	string serverIP = "127.0.0.1";

	SOCKET socket = connectToServer(serverIP, serverPort);

	sendData(socket, data);

	closesocket(socket);
}


SOCKET Server::connectToServer(string serverIP, int serverPort) {
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


void Server::checkNodes() {
	while (!stop) {
		for (int i = 0; i < nodes.size(); i++) {
			NodeData node = nodes.at(i);

			if (!isNodeAlive(node)) {
				cout << "Node " << node.get_server_port() << " is dead" << endl;
				nodes.erase(nodes.begin() + i);

				closesocket(node.get_socket());
			}

			Sleep(1000);
		}
	}
}

bool Server::isNodeAlive(NodeData node) {
	unsigned long long time = Utility::capture_time();
	unsigned long long lastSeen = node.get_last_seen();

	return time - lastSeen < Constants::MAX_TIME_ALIVE;
}

bool Server::nodeInVector(NodeData node)
{
	for (int i = 0; i < nodes.size(); i++) {
		if (nodes.at(i).get_server_port() == node.get_server_port()) {
			return true;
		}
	}

	return false;
}
