#pragma once

#include <string>
#include <iostream>
#include "eccrypto.h"
#include "oids.h"
#include "osrng.h"
#include "eccrypto.h"
#include "oids.h"
#include "osrng.h"
#include "files.h"
#include "oids.h"
#include "eccrypto.h"
// SecByteBlock
#include "secblock.h"

using std::string;
using std::cout;
using std::cerr;
using std::endl;
using CryptoPP::AutoSeededRandomPool;
using CryptoPP::ECIES;
using CryptoPP::ECP;
using CryptoPP::SecBlock;

class ECCHandler
{
public:
	ECCHandler();
	~ECCHandler();

	string encrypt(string cipherText);
	string decrypt(string encryptedText);

	static string encryptECC(string cipherText, string publicKey);
	static string decryptECC(string encryptedText, string privateKey);

	AutoSeededRandomPool rng;
	ECIES<ECP>::Encryptor encryptor;
	ECIES<ECP>::Decryptor decryptor;

	void generateKeyPair();
};

