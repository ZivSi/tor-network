#include "ECCHandler.h"

ECCHandler::ECCHandler()
	: encryptor(CryptoPP::ASN1::secp256r1())
{
	generateKeyPair();
}

ECCHandler::~ECCHandler()
{
}

void ECCHandler::generateKeyPair()
{
	ECIES<ECP>::PrivateKey privateKey;
	privateKey.Initialize(rng, CryptoPP::ASN1::secp256r1());

	ECIES<ECP>::PublicKey publicKey;
	privateKey.MakePublicKey(publicKey);

	encryptor.AccessPublicKey() = publicKey;
	decryptor.AccessPrivateKey() = privateKey;
}

std::string ECCHandler::encrypt(const std::string& plaintext, const std::string& publicKey)
{
	SecByteBlock key;
	StringSource(publicKey, true, new Base64Decoder(new ArraySink(key, key.size())));

	ECIES<ECP>::Encryptor encryptor;
	encryptor.AccessPublicKey().Load(key.data(), key.size());

	std::string ciphertext;
	StringSource(plaintext, true, new PK_EncryptorFilter(rng, encryptor, new StringSink(ciphertext)));

	return ciphertext;
}

std::string ECCHandler::decrypt(const std::string& ciphertext, const std::string& privateKey)
{
	SecByteBlock key;
	StringSource(privateKey, true, new Base64Decoder(new ArraySink(key, key.size())));

	ECIES<ECP>::Decryptor decryptor;
	decryptor.AccessPrivateKey().Load(key.data(), key.size());

	std::string plaintext;
	StringSource(ciphertext, true, new PK_DecryptorFilter(rng, decryptor, new StringSink(plaintext)));

	return plaintext;
}