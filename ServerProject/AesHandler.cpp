//

#include "AesHandler.h"

AesHandler::AesHandler() {
	// Keys are generated in the constructor
}

AesHandler::AesHandler(string key, string iv) {
	selfKeys.initialize(key, iv);
}

AesHandler::~AesHandler() {
}

string AesHandler::encrypt(const string& plaintext, bool log) {
	string ciphertext = AesHandler::encryptAES(plaintext, &selfKeys, log);
	return ciphertext;
}

string AesHandler::decrypt(const string& ciphertext, bool log) {
	string plaintext = AesHandler::decryptAES(ciphertext, &selfKeys, log);
	return plaintext;
}

string AesHandler::encrypt(const string& plaintext) {
	string ciphertext = AesHandler::encryptAES(plaintext, &selfKeys, false);
	return ciphertext;
}

string AesHandler::decrypt(const string& ciphertext) {
	string plaintext = AesHandler::decryptAES(ciphertext, &selfKeys, false);
	return plaintext;
}


AesKey AesHandler::getAesKey()
{
	return this->selfKeys;
}

string AesHandler::encryptAES(const string& plaintext, AesKey* keys, bool log) {
	string ciphertext = "";

	try {

		if (log) {
			cout << "Keys used to encrypt: " << AesKey::SecByteBlockToString(keys->getKey()) << endl;
			cout << "IV used to encrypt: " << AesKey::SecByteBlockToString(keys->getIv()) << endl;

			cout << "Plaintext: " << plaintext << endl;
		}

		CBC_Mode<AES>::Encryption encryption(keys->getKey(), keys->getKey().size(), keys->getIv());
		StringSource(plaintext, true, new StreamTransformationFilter(encryption, new StringSink(ciphertext)));
	}
	catch (const Exception& e) {
		cerr << "Error during encryption: " << e.what() << endl;

		throw std::runtime_error("Error during encryption: " + std::string(e.what()));
	}

	return ciphertext;
}

string AesHandler::decryptAES(const string& ciphertext, AesKey* keys, bool log) {
	string decryptedText = "";

	try {
		if (log) {
			cout << "Keys used to decrypt: " << AesKey::SecByteBlockToString(keys->getKey()) << endl;
			cout << "IV used to decrypt: " << AesKey::SecByteBlockToString(keys->getIv()) << endl;

			cout << "Ciphertext: " << ciphertext << endl;
		}

		CBC_Mode<AES>::Decryption decryption(keys->getKey(), keys->getKey().size(), keys->getIv());
		StringSource(ciphertext, true, new StreamTransformationFilter(decryption, new StringSink(decryptedText)));
	}
	catch (const Exception& e) {
		cerr << "Error during decryption: " << e.what() << endl;

		throw std::runtime_error("Error during decryption: " + std::string(e.what()));
	}

	return decryptedText;
}

string AesHandler::encryptAES(const string& plaintext, AesKey* keys) {
	return encryptAES(plaintext, keys, false);
}

string AesHandler::decryptAES(const string& ciphertext, AesKey* keys) {
	return decryptAES(ciphertext, keys, false);
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

AesKey::AesKey(const string& key, const string& iv) {
	initialize(key, iv);
}

AesKey::AesKey(SecByteBlock key, SecByteBlock iv)
{
	this->key = key;
	this->iv = iv;
}

AesKey::~AesKey() {
}

void AesKey::initialize(const std::string& key, const std::string& iv) {
	// key and iv are null, so we need to initialize them
	this->key.resize(CryptoPP::AES::DEFAULT_KEYLENGTH);
	this->iv.resize(CryptoPP::AES::BLOCKSIZE);

	try {
		CryptoPP::StringSource(key, true, new CryptoPP::HexDecoder(new CryptoPP::ArraySink(reinterpret_cast<CryptoPP::byte*>(&this->key[0]), this->key.size())));
		CryptoPP::StringSource(iv, true, new CryptoPP::HexDecoder(new CryptoPP::ArraySink(reinterpret_cast<CryptoPP::byte*>(&this->iv[0]), this->iv.size())));
	}
	catch (const std::exception& e) {
		throw std::runtime_error("Error initializing AES key and IV: " + std::string(e.what()));
	}
}

SecByteBlock AesKey::getKey() {
	return this->key;
}

SecByteBlock AesKey::getIv() {
	return this->iv;
}

string AesKey::SecByteBlockToString(const SecByteBlock& secByteBlock) {
	return string(reinterpret_cast<const char*>(secByteBlock.BytePtr()), secByteBlock.SizeInBytes());
}

// Helper function to convert string to SecByteBlock
SecByteBlock AesKey::StringToSecByteBlock(const string& str) {
	return SecByteBlock(reinterpret_cast<const byte*>(str.data()), str.size());
}

string AesKey::formatKeyForSending(SecByteBlock key) {
	return SecByteBlockToString(key);
}

SecByteBlock AesKey::reformatKeyForReceiving(const std::string& key) {
	return StringToSecByteBlock(key);
}

string AesKey::serializeKey()
{
	string keySerialized = formatKeyForSending(getKey());
	string ivSerialized = formatKeyForSending(getIv());

	return keySerialized + ivSerialized;
}


AesKey AesKey::decryptedAESKeysToPair(string decryptedAESKeys) {
	string extractedAes = "";
	string extractedIv = "";

	Utility::extractAESKey(decryptedAESKeys, extractedAes);
	Utility::extractAESIv(decryptedAESKeys, extractedIv);

	SecByteBlock aesKey = AesKey::StringToSecByteBlock(extractedAes);
	SecByteBlock aesIv = AesKey::StringToSecByteBlock(extractedIv);

	return AesKey(aesKey, aesIv);
}