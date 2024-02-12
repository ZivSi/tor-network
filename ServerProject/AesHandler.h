#pragma once

#include "Constants.h"
#include "Utility.h"
#include <aes.h>
#include <base64.h>
#include <filters.h>
#include <hex.h>
#include <iostream>
#include <modes.h>
#include <osrng.h>
#include <secblock.h>
#include <string>


using CryptoPP::AES;
using CryptoPP::CBC_Mode;
using CryptoPP::StringSource;
using CryptoPP::StreamTransformationFilter;
using CryptoPP::StringSink;
using CryptoPP::Exception;
using CryptoPP::AutoSeededRandomPool;
using CryptoPP::byte;
using CryptoPP::SecByteBlock;
using CryptoPP::ArraySink;
using CryptoPP::HexEncoder;
using CryptoPP::HexDecoder;
using CryptoPP::Base64Encoder;
using CryptoPP::Base64Decoder;
using std::cerr;
using std::endl;
using std::cout;
using std::string;

using namespace Constants;

#define KEY_SIZE_BITS 128
#define BLOCK_SIZE_BITS 128

class AesKey {
private:
	SecByteBlock key;
	SecByteBlock iv;

public:
	AesKey();
	AesKey(const string& key, const string& iv);
	AesKey(SecByteBlock key, SecByteBlock iv);
	~AesKey();

	void initialize(const std::string& key, const std::string& iv);

	SecByteBlock getKey() const;
	SecByteBlock getIv() const;

	static CryptoPP::SecByteBlock StringToSecByteBlock(const string& str);
	static string SecByteBlockToString(const CryptoPP::SecByteBlock& secByteBlock);
	static AesKey decryptedAESKeysToPair(string decryptedAESKeys);

	string formatKeyForSending(CryptoPP::SecByteBlock key);
	SecByteBlock reformatKeyForReceiving(const std::string& key);

	string serializeKey();
};

class AesHandler {
public:
	AesHandler();
	AesHandler(string key, string iv);
	~AesHandler();

	string encrypt(const string& plaintext);
	string decrypt(const string& ciphertext);

	string encrypt(const string& plaintext, bool log);
	string decrypt(const string& ciphertext, bool log);

	AesKey getAesKey() const;

	static string encryptAES(const string& plaintext, AesKey* keys, bool log);
	static string decryptAES(const string& ciphertext, AesKey* keys, bool log);

	static string encryptAES(const string& plaintext, AesKey* keys);
	static string decryptAES(const string& ciphertext, AesKey* keys);

private:
	AesKey selfKeys;
};