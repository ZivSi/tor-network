#include "ECCHandler.h"

ECCHandler::ECCHandler()
{
	generateKeyPair(&rng);
}

ECCHandler::ECCHandler(const std::string& serializedPublicKey)
{
	CryptoPP::ECIES<ECP>::PublicKey pubKey;

	try
	{

		StringSource ss(serializedPublicKey, true);
		pubKey.Load(ss);

		cout << "Loaded key in size of: " << serializedPublicKey.size() << endl;
		cout << "Loaded key in sizeof of: " << sizeof(serializedPublicKey) << endl;
	}
	catch (const CryptoPP::Exception& e)
	{
		throw e;
	}

	this->encryptor = ECIES<ECP>::Encryptor(pubKey);
}

ECCHandler::~ECCHandler()
{
}

ECIES<ECP>::Encryptor ECCHandler::buildEncryptor(string serializedPublicKey)
{
	CryptoPP::ECIES<ECP>::PublicKey pubKey;

	try
	{
		StringSource ss(serializedPublicKey, true);
		pubKey.Load(ss);
	}
	catch (const CryptoPP::Exception& e)
	{
		cerr << "Error loading public key: " << e.what() << std::endl;
		throw e;
	}

	return ECIES<ECP>::Encryptor(pubKey);
}

void ECCHandler::generateKeyPair(AutoSeededRandomPool* rng) {

	privateKey.Initialize(*rng, CryptoPP::ASN1::secp256r1());

	privateKey.MakePublicKey(this->publicKey);

	this->encryptor = ECIES<ECP>::Encryptor(publicKey);
	this->decryptor = ECIES<ECP>::Decryptor(privateKey);
}

string ECCHandler::encrypt(const string& plaintext)
{
	return encrypt(plaintext, &encryptor, &rng);
}

string ECCHandler::decrypt(const string& ciphertext)
{
	return decrypt(ciphertext, &decryptor, &rng);
}

string ECCHandler::encrypt(const string& plaintext, ECIES<ECP>::Encryptor* encryptor, AutoSeededRandomPool* rng)
{
	string ciphertext;

	StringSource(plaintext, true,
		new PK_EncryptorFilter(*rng, *encryptor,
			new StringSink(ciphertext)
		)
	);

	return ciphertext;
}

string ECCHandler::decrypt(const string& ciphertext, ECIES<ECP>::Decryptor* decryptor, AutoSeededRandomPool* rng)
{
	string decryptedText;

	StringSource(ciphertext, true,
		new PK_DecryptorFilter(*rng, *decryptor,
			new StringSink(decryptedText)
		)
	);

	return decryptedText;
}

void ECCHandler::setPublicKey(const string& publicKey)
{
	CryptoPP::ECIES<ECP>::PublicKey pubKey;
	StringSource ss(publicKey, true);
	pubKey.Load(ss);

	this->encryptor = ECIES<ECP>::Encryptor(pubKey);
}

AutoSeededRandomPool* ECCHandler::getRNG()
{
	return &rng;
}

ECIES<ECP>::Encryptor* ECCHandler::getEncryptor()
{
	return &encryptor;
}

ECIES<ECP>::Decryptor* ECCHandler::getDecryptor()
{
	return &decryptor;
}

string ECCHandler::serializeKey() {
	std::string serializedPublicKey;
	this->publicKey.Save(StringSink(serializedPublicKey).Ref());

	return serializedPublicKey;
}


ECCHandler& ECCHandler::operator=(const ECCHandler& other)
{
	if (this != &other)
	{
		this->publicKey = other.publicKey;
		this->privateKey = other.privateKey;
		this->encryptor = other.encryptor;
		this->decryptor = other.decryptor;
	}

	return *this;
}
