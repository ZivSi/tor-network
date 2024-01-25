#pragma once

#include <iostream>
#include <string>
#include <eccrypto.h>
#include <oids.h>
#include <osrng.h>
#include <base64.h>
#include <files.h>
#include <secblock.h>
#include <specstrings.h>
// BufferedTransformation
#include <cryptlib.h>

#include "Logger.h"

using CryptoPP::ECP;
using CryptoPP::ECIES;
using CryptoPP::AutoSeededRandomPool;
using CryptoPP::StringSource;
using CryptoPP::StringSink;
using CryptoPP::Base64Encoder;
using CryptoPP::Base64Decoder;
using CryptoPP::ArraySink;
using CryptoPP::SecByteBlock;
using CryptoPP::PK_EncryptorFilter;
using CryptoPP::PK_DecryptorFilter;
using CryptoPP::BufferedTransformation;
using std::string;
using std::cout;
using std::endl;
using std::cerr;


class ECCHandler
{
public:
	ECCHandler();
	ECCHandler(const string& serializedPublicKey);
	~ECCHandler();

	// Replace the constrauctor
	void initialize(const string& serializedPublicKey);

	string encrypt(const string& plaintext);
	string decrypt(const string& ciphertext);

	void setPublicKey(const string& publicKey);

	AutoSeededRandomPool* getRNG();
	ECIES<ECP>::Encryptor* getEncryptor();
	ECIES<ECP>::Decryptor* getDecryptor();

	string serializeKey();

	void generateKeyPair(AutoSeededRandomPool* rng);

	ECCHandler& operator=(const ECCHandler& other);


	static ECIES<ECP>::Encryptor buildEncryptor(string serializedPublicKey);
	static string encrypt(const string& plaintext, ECIES<ECP>::Encryptor* encryptor, AutoSeededRandomPool* rng);
	static string decrypt(const string& ciphertext, ECIES<ECP>::Decryptor* decryptor, AutoSeededRandomPool* rng);


private:
	AutoSeededRandomPool rng;

	ECIES<ECP>::PublicKey publicKey;
	ECIES<ECP>::PrivateKey privateKey;

	ECIES<ECP>::Encryptor encryptor;
	ECIES<ECP>::Decryptor decryptor;
};
