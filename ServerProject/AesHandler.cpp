//

#include "AesHandler.h"

AesHandler::AesHandler() {
	// Keys will be generated in the ctor of AesKey
}

AesHandler::AesHandler(string key, string iv) {
	selfKeys.initialize(key, iv);
}

AesHandler::~AesHandler() {
}

string AesHandler::encrypt(const string& plaintext) {
	string ciphertext = AesHandler::encryptAES(plaintext, selfKeys);
	return ciphertext;
}

string AesHandler::decrypt(const string& ciphertext) {
	string plaintext = AesHandler::decryptAES(ciphertext, selfKeys);
	return plaintext;
}

string AesHandler::getKeys()
{
	string formatted = AesHandler::formatKeyForSending(selfKeys.getKey());
	formatted += SPLITER;
	formatted += AesHandler::formatKeyForSending(selfKeys.getIv());

	return formatted;
}

SecByteBlock AesHandler::getKey() {
	return this->selfKeys.getKey();
}

SecByteBlock AesHandler::getIv() {
	return this->selfKeys.getIv();
}

string AesHandler::encryptAES(const string& plaintext, AesKey keys) {
	string ciphertext;

	try {
		CBC_Mode<AES>::Encryption encryption(keys.getKey(), keys.getKey().size(), keys.getIv());
		StringSource(plaintext, true, new StreamTransformationFilter(encryption, new StringSink(ciphertext)));
	}
	catch (const Exception& e) {
		cerr << "Error during encryption: " << e.what() << endl;
	}

	return ciphertext;
}

string AesHandler::decryptAES(const string& ciphertext, AesKey keys) {
	string decryptedText;

	try {
		CBC_Mode<AES>::Decryption decryption(keys.getKey(), keys.getKey().size(), keys.getIv());
		StringSource(ciphertext, true, new StreamTransformationFilter(decryption, new StringSink(decryptedText)));
	}
	catch (const Exception& e) {
		cerr << "Error during decryption: " << e.what() << endl;
	}

	return decryptedText;
}

string AesHandler::formatKeyForSending(CryptoPP::SecByteBlock key) {
	return SecByteBlockToString(key);
}

SecByteBlock AesHandler::reformatKeyForReceiving(const std::string& key) {
	return StringToSecByteBlock(key);
}

string AesHandler::serializeKey()
{
	string keySerialized = formatKeyForSending(getKey());
	string ivSerialized = formatKeyForSending(getIv());

	return keySerialized + ivSerialized;
}


string AesHandler::SecByteBlockToString(const CryptoPP::SecByteBlock& secByteBlock) {
	return string(reinterpret_cast<const char*>(secByteBlock.BytePtr()), secByteBlock.SizeInBytes());
}

// Helper function to convert string to SecByteBlock
CryptoPP::SecByteBlock AesHandler::StringToSecByteBlock(const string& str) {
	return CryptoPP::SecByteBlock(reinterpret_cast<const byte*>(str.data()), str.size());
}

AesKey::AesKey() {
	// Generate a random key and iv
	AutoSeededRandomPool prng;

	// Set the key size (16 bytes for AES-128, 24 bytes for AES-192, 32 bytes for AES-256)
	const size_t keySize = CryptoPP::AES::DEFAULT_KEYLENGTH;
	this->key.resize(keySize);
	prng.GenerateBlock(this->key, keySize);

	// Set the IV size (16 bytes for AES)
	const size_t ivSize = CryptoPP::AES::BLOCKSIZE;
	this->iv.resize(ivSize);
	prng.GenerateBlock(this->iv, ivSize);
}

AesKey::AesKey(string key, string iv) {
	initialize(key, iv);
}

AesKey::AesKey(SecByteBlock key, SecByteBlock iv)
{
	this->key = key;
	this->iv = iv;
}

AesKey::~AesKey() {
}

void AesKey::initialize(string key, string iv)
{
	StringSource(key, true, new HexDecoder(new ArraySink(this->key, this->key.size())));
	StringSource(iv, true, new HexDecoder(new ArraySink(this->iv, this->iv.size())));
}

SecByteBlock AesKey::getKey() {
	return this->key;
}

SecByteBlock AesKey::getIv() {
	return this->iv;
}
