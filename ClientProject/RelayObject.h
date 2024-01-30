#pragma once

#include "AesHandler.h"

class RelayObject
{
private:
	unsigned short port;
	AesKey aesKey;

public:
	RelayObject(unsigned short port, AesKey aesKeys);
	~RelayObject();

	unsigned short getPort();
	AesKey getAesKeys();
};

