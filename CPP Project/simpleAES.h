#include <iostream>
#include <string>
#include <aes.h>
#include <modes.h>
#include <filters.h>
#include <osrng.h>
#include <hex.h>
#include <secblock.h>


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

#define KEY_SIZE_BITS 128
#define BLOCK_SIZE_BITS 128

class simpleAES {
public:
	simpleAES();
	simpleAES(string key, string iv);
	~simpleAES();

	string encrypt(const string& plaintext);
	string decrypt(const string& ciphertext);


	static string encryptAES(const string& plaintext, SecByteBlock key, SecByteBlock iv);
	static string decryptAES(const string& ciphertext, SecByteBlock key, SecByteBlock iv);
	static string format_key_for_sending(SecByteBlock key);
	static SecByteBlock reformat_key_for_receiving(string& key);


private:
	SecByteBlock key;
	SecByteBlock iv;
};