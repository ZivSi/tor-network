#include "RelayObject.h"

RelayObject::RelayObject(unsigned short port, AesKey aesKeys, ECCHandler* eccHandler, string conversationId) {
	this->port = port;
	this->aesKey = aesKeys;
	this->eccHandler = eccHandler;
	this->conversationId = conversationId;

	this->conversationIdEncrypted = this->eccHandler->encrypt(conversationId);
}

RelayObject::RelayObject(unsigned short port)
{
	this->port = port;
}

RelayObject::~RelayObject() {

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
