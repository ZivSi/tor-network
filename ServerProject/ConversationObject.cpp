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
}

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

void ConversationObject::setPrvNode(SOCKET prvNode) {
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

bool ConversationObject::isEmpty() const
{
	return conversationId == "";
}

void ConversationObject::setAsExitNode()
{
	this->exitNode = true;
}

bool ConversationObject::isExitNode() const
{
	return this->exitNode;
}

bool ConversationObject::isTooOld() const
{
	return Utility::capture_time() - creationTime > Constants::CONVERSATION_TIMEOUT;
}
