#pragma once

#include <iostream>
#include <string>
#include <aes.h>
#include <modes.h>
#include <filters.h>
#include <osrng.h>
#include <hex.h>
#include <secblock.h>
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
using std::cerr;
using std::endl;
using std::cout;
using std::string;

using namespace Constants;

#define KEY_SIZE_BITS 128
#define BLOCK_SIZE_BITS 128

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


	static string encryptAES(const string& plaintext, SecByteBlock key, SecByteBlock iv);
	static string decryptAES(const string& ciphertext, SecByteBlock key, SecByteBlock iv);
	static string format_key_for_sending(SecByteBlock key);
	static SecByteBlock reformat_key_for_receiving(string& key);
	static string SecByteBlockToString(const CryptoPP::SecByteBlock& secByteBlock);
	static CryptoPP::SecByteBlock StringToSecByteBlock(const string& str);

	

private:
	SecByteBlock key;
	SecByteBlock iv;
};