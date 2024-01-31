#include "ConversationObject.h"

string ConversationObject::LETTERS = "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

ConversationObject::ConversationObject() {
	this->prvNode = INVALID_SOCKET;
	this->nxtNode = INVALID_SOCKET;
	this->conversationId = "";
}

ConversationObject::ConversationObject(SOCKET prvNode, SOCKET nxtNode, string conversationId, AesKey key) {
	this->prvNode = prvNode;
	this->nxtNode = nxtNode;
	this->conversationId = conversationId;
	this->key = key;
}

ConversationObject::ConversationObject(string conversationId, AesKey key) {
	this->prvNode = INVALID_SOCKET;
	this->nxtNode = INVALID_SOCKET;
	this->conversationId = conversationId;
	this->key = key;
}

ConversationObject::~ConversationObject() {
}

SOCKET ConversationObject::getPrvNode() {
	return this->prvNode;
}

SOCKET ConversationObject::getNxtNode() {
	return this->nxtNode;
}

unsigned short ConversationObject::getPrvPort()
{
	return this->prvPort;
}

unsigned short ConversationObject::getNxtPort()
{
	return this->nxtPort;
}


string ConversationObject::getConversationId() {
	return this->conversationId;
}

AesKey ConversationObject::getKey() {
	return this->key;
}

void ConversationObject::setPrvNode(SOCKET prvNode) {
	this->prvNode = prvNode;
}

void ConversationObject::setNxtNode(SOCKET nxtNode) {
	this->nxtNode = nxtNode;
}

void ConversationObject::setPrvPort(unsigned short prvPort)
{
	this->prvPort = prvPort;
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

bool ConversationObject::isEmpty()
{
	return conversationId == "";
}
