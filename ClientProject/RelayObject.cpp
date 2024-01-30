#include "RelayObject.h"

RelayObject::RelayObject(unsigned short port, AesKey aesKeys)
{
    this->port = port;
    this->aesKey = aesKeys;
}

RelayObject::~RelayObject() {

}

unsigned short RelayObject::getPort()
{
    return this->port;
}

AesKey RelayObject::getAesKeys() {
    return this->aesKey;
}