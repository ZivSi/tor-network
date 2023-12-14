#include "simpleAES.h"

simpleAES::simpleAES() {
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

simpleAES::simpleAES(string key, string iv) {
	// Assume key and iv are hex-encoded strings
	StringSource(key, true, new HexDecoder(new ArraySink(this->key, this->key.size())));
	StringSource(iv, true, new HexDecoder(new ArraySink(this->iv, this->iv.size())));
}

simpleAES::~simpleAES() {
}

string simpleAES::encrypt(const string& plaintext) {
	string ciphertext = simpleAES::encryptAES(plaintext, this->key, this->iv);
	return ciphertext;
}

string simpleAES::decrypt(const string& ciphertext) {
	string plaintext = simpleAES::decryptAES(ciphertext, this->key, this->iv);
	return plaintext;
}

string simpleAES::encryptAES(const string& plaintext, SecByteBlock key, SecByteBlock iv) {
	string ciphertext;

	try {
		CBC_Mode<AES>::Encryption encryption(key, key.size(), iv);
		StringSource(plaintext, true, new StreamTransformationFilter(encryption, new StringSink(ciphertext)));
	}
	catch (const Exception& e) {
		cerr << "Error during encryption: " << e.what() << endl;
	}

	return ciphertext;
}

string simpleAES::decryptAES(const string& ciphertext, SecByteBlock key, SecByteBlock iv) {
	string decryptedText;

	try {
		CBC_Mode<AES>::Decryption decryption(key, key.size(), iv);
		StringSource(ciphertext, true, new StreamTransformationFilter(decryption, new StringSink(decryptedText)));
	}
	catch (const Exception& e) {
		cerr << "Error during decryption: " << e.what() << endl;
	}

	return decryptedText;
}

string simpleAES::format_key_for_sending(SecByteBlock key) {
	string formattedKey;

	CryptoPP::HexEncoder encoder(new CryptoPP::StringSink(formattedKey));
	encoder.Put(key, key.size());
	encoder.MessageEnd();

	return formattedKey;
}

SecByteBlock simpleAES::reformat_key_for_receiving(string& key) {
	SecByteBlock formattedKey;

	CryptoPP::HexDecoder decoder;
	decoder.Put(reinterpret_cast<const unsigned char*>(key.c_str()), key.size());
	decoder.MessageEnd();

	size_t size = decoder.MaxRetrievable();
	if (size && size <= CryptoPP::AES::MAX_KEYLENGTH) {
		formattedKey.resize(size);
		decoder.Get(&formattedKey[0], formattedKey.size());
	}

	return formattedKey;
}