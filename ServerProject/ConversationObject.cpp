#include "ConversationObject.h"

string ConversationObject::LETTERS = "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

ConversationObject::ConversationObject() : nxtPort(0), prvPort(0) {
	this->prvNodeSocket = -1;
	this->nxtNode = nullptr;
	this->conversationId = "";

	this->creationTime = Utility::capture_time();
}

ConversationObject::ConversationObject(SOCKET prvNode, ClientConnection* nxtNode, string conversationId, AesKey key) : nxtPort(0), prvPort(0) {
	this->prvNodeSocket = prvNode;
	this->nxtNode = nxtNode;
	this->conversationId = conversationId;
	this->key = key;

	this->creationTime = Utility::capture_time();
}

ConversationObject::ConversationObject(string conversationId, AesKey key) : nxtPort(0), prvPort(0) {
	this->prvNodeSocket = -1;
	this->nxtNode = nullptr;
	this->conversationId = conversationId;
	this->key = key;

	this->creationTime = Utility::capture_time();
}

ConversationObject::~ConversationObject() {
	if (this->nxtNode != nullptr) {
		delete this->nxtNode;
	}

	if (this->prvNodeSocket != -1) {
		closesocket(this->prvNodeSocket);
	}

	for (auto it = this->destinationMap.begin(); it != this->destinationMap.end(); it++) {
		delete it->second;
	}

	this->destinationMap.clear();

	while (!messageQueue.empty()) {
		messageQueue.pop();
	}
}

// ----------------- Getters -----------------
SOCKET ConversationObject::getPrvNodeSOCKET() const {
	return this->prvNodeSocket;
}

ClientConnection* ConversationObject::getNxtNode() {
	return this->nxtNode;
}

string ConversationObject::getPrvIP() const
{
	return this->prvIP;
}

unsigned short ConversationObject::getPrvPort()
{
	return this->prvPort;
}

string ConversationObject::getNxtIP() const
{
	return this->nxtIP;
}

unsigned short ConversationObject::getNxtPort() const
{
	return this->nxtPort;
}

string ConversationObject::getConversationId() {
	return this->conversationId;
}

AesKey* ConversationObject::getKey() {
	return &(this->key);
}

bool ConversationObject::isEmpty() const
{
	return conversationId == "";
}

bool ConversationObject::isExitNode() const
{
	return this->exitNode;
}

bool ConversationObject::isTooOld() const
{
	return Utility::capture_time() - creationTime > Constants::PATH_TIMEOUT;
}

string DestinationData::getDestinationIP()
{
	return this->destinationIP;
}

unsigned short DestinationData::getDestinationPort()
{
	return this->destinationPort;
}

string DestinationData::getData()
{
	return this->data;
}

string ConnectionPair::getIP() const
{
	return this->ip;
}

unsigned short ConnectionPair::getPort() const
{
	return this->port;
}


// ----------------- Setters -----------------
void ConversationObject::setPrvNodeSOCKET(SOCKET prvNode) {
	this->prvNodeSocket = prvNode;
}

void ConversationObject::setNxtNode(ClientConnection* nxtNode) {
	this->nxtNode = nxtNode;
}

void ConversationObject::setPrvIP(string prvIP)
{
	this->prvIP = prvIP;
}

void ConversationObject::setPrvPort(unsigned short prvPort)
{
	this->prvPort = prvPort;
}

void ConversationObject::setNxtIP(string nxtIP)
{
	this->nxtIP = nxtIP;
}

void ConversationObject::setNxtPort(unsigned short nxtPort)
{
	this->nxtPort = nxtPort;
}

void ConversationObject::setConversationId(string conversationId) {
	this->conversationId = conversationId;
}

void ConversationObject::setKey(AesKey key) {
	this->key = key;
}

void ConversationObject::setAsExitNode()
{
	this->exitNode = true;
}


// ----------------- Map related -----------------
ClientConnection* ConversationObject::addActiveConnection(DestinationData destinationData)
{
	ClientConnection* newConnection;

	try {
		newConnection = destinationData.createConnection();
	}
	catch (...) {
		throw std::runtime_error("Could not create connection to destination");
	}

	this->destinationMap[ConnectionPair(destinationData.getDestinationIP(), destinationData.getDestinationPort())] = newConnection;

	return newConnection;
}

void ConversationObject::removeActiveConnection(DestinationData destinationData)
{
	auto it = this->destinationMap.find(ConnectionPair(destinationData.getDestinationIP(), destinationData.getDestinationPort()));

	if (it != this->destinationMap.end()) {
		delete it->second;
		this->destinationMap.erase(it);

		return;
	}

	cerr << "Could not find connection to remove (IP: " << destinationData.getDestinationIP() << ", Port: " << destinationData.getDestinationPort() << ")" << endl;
}

ClientConnection* ConversationObject::getActiveConnection(DestinationData destinationData)
{
	auto it = this->destinationMap.find(ConnectionPair(destinationData.getDestinationIP(), destinationData.getDestinationPort()));

	if (it != this->destinationMap.end()) {
		return it->second;
	}

	return nullptr;
}

bool ConversationObject::isDestinationActive(DestinationData destinationData)
{
	auto it = this->destinationMap.find(ConnectionPair(destinationData.getDestinationIP(), destinationData.getDestinationPort()));

	bool found = it != this->destinationMap.end();

	if (!found) {
		return false;
	}

	return it->second->isConnectionActive();
}

ClientConnection* DestinationData::createConnection()
{
	return new ClientConnection(this->destinationIP, this->destinationPort);
}

// ----------------- Utility -----------------
// TODO: make sure that the generated ID is unique
string ConversationObject::generateID() {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(0, LETTERS.size() - 1);

	std::string uuid;
	uuid.reserve(UUID_LEN);

	for (int i = 0; i < UUID_LEN; ++i) {
		int index = dis(gen);
		uuid += LETTERS[index];
	}

	return uuid;
}

string ConnectionPair::toString() const
{
	return ip + ":" + to_string(port);
}

std::ostream& operator<<(std::ostream& os, const ConnectionPair& obj)
{
	os << obj.ip << ":" << obj.port;

	return os;
}


// ----------------- Queue Related -----------------
bool ConversationObject::isQueueEmpty() const
{
	return messageQueue.empty();
}

size_t ConversationObject::getQueueSize() const
{
	return messageQueue.size();
}

void ConversationObject::addMessage(string message)
{
	messageQueue.push(message);
}

string ConversationObject::getFirstMessage()
{
	string message = messageQueue.front();
	messageQueue.pop();

	return message;
}

void ConversationObject::collectMessages()
{
	if (!isExitNode()) {
		cerr << "You are not supposed to collect messages in this conversation object. You are not an exit node" << endl;

		return;
	}


	for (auto it = this->destinationMap.begin(); it != this->destinationMap.end(); it++) {
		ClientConnection* connection = it->second;
		string receivedData = "";

		try
		{
			receivedData = connection->receiveDataFromTcp();
		}
		catch (std::runtime_error e)
		{
			// Host unreachable so we remove the connection
			cerr << "Host: " << it->first << " is unreachable. Removing connection" << endl;

			delete it->second;
			this->destinationMap.erase(it);
		}

		if (receivedData != "") { // The data is legit and not some noise
			receivedData = connection->getIP() + SPLITER + to_string(connection->getPort()) + SPLITER + receivedData;
			string encryptedData = AesHandler::encryptAES(receivedData, &(this->key));

			this->addMessage(encryptedData);
		}
	}
}


// ----------------- Constructor and Destructor (DestinationData and ConnectionPair) -----------------
DestinationData::DestinationData(string destinationIP, unsigned short destinationPort, string data)
{
	this->destinationIP = destinationIP;
	this->destinationPort = destinationPort;
	this->data = data;
}

DestinationData::DestinationData(string decryptedData)
{
	vector<string> destinationProperties = Utility::splitString(decryptedData, SPLITER);

	this->destinationIP = destinationProperties[IP_INDEX];
	this->destinationPort = static_cast<unsigned short>(stoi(destinationProperties[PORT_INDEX]));
	this->data = destinationProperties[DATA_INDEX];
}

DestinationData::~DestinationData()
{
}

ConnectionPair::ConnectionPair(string ip, unsigned short port)
{
	this->ip = ip;
	this->port = port;
}

ConnectionPair::~ConnectionPair()
{
}

// ----------------- Overloaded Operators -----------------
bool ConnectionPair::operator==(const ConnectionPair& other) const
{
	return this->ip == other.ip && this->port == other.port;
}

bool ConnectionPair::operator<(const ConnectionPair& other) const
{
	if (this->ip == other.ip) {
		return this->port < other.port;
	}

	return this->ip < other.ip;
}