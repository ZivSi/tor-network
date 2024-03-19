#include "RelayObject.h"

RelayObject::RelayObject(string ip, unsigned short port, AesKey aesKeys, ECCHandler* eccHandler, string conversationId) {
	this->ip = ip;
	this->port = port;
	this->aesKey = aesKeys;
	this->eccHandler = eccHandler;
	this->conversationId = conversationId;

	this->conversationIdEncrypted = this->eccHandler->encrypt(conversationId);
}

RelayObject::RelayObject(string ip, unsigned short port)
{
	this->ip = ip;
	this->port = port;
}

RelayObject::~RelayObject() {

}

string RelayObject::getIp()
{
	return this->ip;
}

unsigned short RelayObject::getPort()
{
	return this->port;
}

AesKey* RelayObject::getAesKeys() {
	return &(this->aesKey);
}

ECCHandler* RelayObject::getEccHandler()
{
	return eccHandler;
}

string RelayObject::getConversationId()
{
	return this->conversationId;
}

string RelayObject::getConversationIdEncrypted()
{
	return this->conversationIdEncrypted;
}

void RelayObject::setAesKeys(AesKey aesKeys)
{

	this->aesKey = aesKeys;
}

void RelayObject::setEccHandler(ECCHandler* eccHandler)
{
	this->eccHandler = eccHandler;
}


void RelayObject::setConversationId(string conversationId)
{
	this->conversationId = conversationId;
	this->conversationIdEncrypted = eccHandler->encrypt(conversationId);
}

string RelayObject::toString()
{
	return this->ip + ":" + std::to_string(this->port);
}

RelayProperties::RelayProperties(string ip, unsigned short port)
{
	this->ip = ip;
	this->port = port;
}

RelayProperties::RelayProperties()
{
}

RelayProperties::~RelayProperties()
{
}

string RelayProperties::getIp()
{
	return this->ip;
}

unsigned short RelayProperties::getPort()
{
	return this->port;
}

void RelayProperties::setIp(string ip)
{
	this->ip = ip;
}

void RelayProperties::setPort(unsigned short port)
{
	this->port = port;
}
