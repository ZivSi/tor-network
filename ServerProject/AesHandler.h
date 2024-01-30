#pragma once

#include <iostream>
#include <string>
#include <aes.h>
#include <modes.h>
#include <filters.h>
#include <osrng.h>
#include <hex.h>
#include <secblock.h>
#include <base64.h>
#include "Constants.h"


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
	AesKey(string key, string iv);
	AesKey(SecByteBlock key, SecByteBlock iv);
	~AesKey();

	void initialize(string key, string iv);

	SecByteBlock getKey();
	SecByteBlock getIv();
};

class AesHandler {
public:
	AesHandler();
	AesHandler(string key, string iv);
	~AesHandler();

	string encrypt(const string& plaintext);
	string decrypt(const string& ciphertext);

	string getKeys();
	SecByteBlock getKey();
	SecByteBlock getIv();

	string formatKeyForSending(SecByteBlock key);
	SecByteBlock reformatKeyForReceiving(const std::string& key);

	string serializeKey();

	static string encryptAES(const string& plaintext, AesKey keys);
	static string decryptAES(const string& ciphertext, AesKey keys);
	static string SecByteBlockToString(const CryptoPP::SecByteBlock& secByteBlock);
	static CryptoPP::SecByteBlock StringToSecByteBlock(const string& str);

private:
	AesKey selfKeys;
};