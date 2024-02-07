#include "NodeData.h"

NodeData::NodeData(unsigned short port) {
	this->port = port;
	this->lastAliveMessageTime = Utility::capture_time();
	this->aliveMessagesCount = 0;
	this->averageResponseTime = 0;
}

NodeData::NodeData(unsigned short port, string serializedPublicKey) : ecchandler(serializedPublicKey) {
	this->port = port;
	this->lastAliveMessageTime = Utility::capture_time();
	this->aliveMessagesCount = 0;
	this->averageResponseTime = 0;
}

NodeData::NodeData(unsigned short port, string serializedPublicKey, unsigned long long lastAliveMessageTime, unsigned long aliveMessagesCount, unsigned long averageResponseTime) {
	this->port = port;
	this->lastAliveMessageTime = lastAliveMessageTime;
	this->aliveMessagesCount = aliveMessagesCount;
	this->averageResponseTime = averageResponseTime;
}

NodeData::NodeData() {
	this->port = 0;
	this->lastAliveMessageTime = 0;
	this->aliveMessagesCount = 0;
	this->averageResponseTime = 0;
}

NodeData::~NodeData() {
	// Destructor implementation (if needed)
}

unsigned short NodeData::getPort() {
	return this->port;
}

ECCHandler* NodeData::getECCHandler() {
	return &this->ecchandler;
}

unsigned long long NodeData::getLastAliveMessageTime() {
	return this->lastAliveMessageTime;
}

unsigned long NodeData::getAliveMessagesCount() {
	return this->aliveMessagesCount;
}

unsigned long NodeData::getAverageResponseTime() {
	return this->averageResponseTime;
}

void NodeData::setPort(unsigned short port) {
	this->port = port;
}

void NodeData::setECCKey(string serializedPublicKey) {
	this->ecchandler = ECCHandler(serializedPublicKey);
}

void NodeData::setLastAliveMessageTime(unsigned long long lastAliveMessageTime) {
	this->lastAliveMessageTime = lastAliveMessageTime;
}

void NodeData::updateLastAliveMessageTime() {
	this->lastAliveMessageTime = Utility::capture_time();

	this->aliveMessagesCount++;
}

void NodeData::setAliveMessagesCount(unsigned long aliveMessagesCount) {
	this->aliveMessagesCount = aliveMessagesCount;
}

void NodeData::setAverageResponseTime(unsigned long averageResponseTime) {
	this->averageResponseTime = averageResponseTime;
}

string NodeData::encrypt(string plaintext)
{
	return this->ecchandler.encrypt(plaintext);
}

string NodeData::toString() {
	return "Node: " + to_string(this->port) + " | Last alive message time: " + to_string(this->lastAliveMessageTime) + " | Alive messages count: " + to_string(this->aliveMessagesCount) + " | Average response time: " + to_string(this->averageResponseTime);
}

vector<unsigned char> NodeData::toSend() {
	vector<unsigned char> result;
	
	// Port
	result.push_back((this->port >> 8) & 0xFF);
	result.push_back(this->port & 0xFF);

	// Serialized public key
	string serializedPublicKey = this->ecchandler.serializeKey();
	for (int i = 0; i < serializedPublicKey.length(); i++) {
		result.push_back(serializedPublicKey[i]);
	}	

	return result;
}

bool NodeData::isEmpty()
{
	return this->port == 0;
}

NodeData::operator int() const {
	return this->port;
}

bool NodeData::operator==(const NodeData& other) const {
	return this->port == other.port;
}
