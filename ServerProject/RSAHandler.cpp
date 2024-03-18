#include "RSAHandler.h"
#include <cmath>
#include <cstdlib>
#include <numeric>

RSAHandler::RSAHandler(int key_size) {
	int64_t biggest_num = pow(2, key_size - 1);

	p = generate_prime(biggest_num);
	q = generate_prime(biggest_num);

	modulus = p * q;
	t = totient(modulus);

	publicKey = generate_e(t);
	privateKey = calculate_d(publicKey, t);
}

void RSAHandler::setClientPublicKey(int64_t clientPublicKey) {
	this->clientPublicKey = clientPublicKey;
}

void RSAHandler::setClientModulus(int64_t clientModulus) {
	this->clientModulus = clientModulus;
}

int64_t RSAHandler::getPublicKey() {
	return publicKey;
}

int64_t RSAHandler::getModulus() {
	return modulus;
}

int64_t RSAHandler::getPrivateKey() {
	return privateKey;
}

std::vector<int64_t> RSAHandler::encrypt(string m) {
	if (clientPublicKey == 0 || clientModulus == 0) {
		throw std::runtime_error("Client public key or modulus not set");
	}

	std::vector<int64_t> encryptedMessage;

	for (char c : m) {
		int64_t encryptedChar = mod_pow(c, clientPublicKey, clientModulus);
		encryptedMessage.push_back(encryptedChar);
	}

	return encryptedMessage;
}

string RSAHandler::encryptToString(std::string m) {
	std::vector<int64_t> encryptedList = encrypt(m);
	string result = "[";

	if (!encryptedList.empty()) {
		result += std::to_string(encryptedList[0]);
		for (size_t i = 1; i < encryptedList.size(); ++i) {
			result += ", " + std::to_string(encryptedList[i]);
		}
	}

	result += "]";
	return result;
}

std::string RSAHandler::decrypt(const std::vector<int64_t>& encrypted) {
	std::string decryptedMessage;

	for (int64_t encryptedChar : encrypted) {
		char decryptedChar = static_cast<char>(mod_pow(encryptedChar, privateKey, modulus));
		decryptedMessage += decryptedChar;
	}

	return decryptedMessage;
}

string RSAHandler::decrypt(const std::string& encrypted) {
	// Extract the encrypted numbers from the string
	std::vector<int64_t> encryptedList = parseEncryptedString(encrypted);
	// Decrypt the encrypted numbers and return the result
	return decrypt(encryptedList);
}

std::vector<int64_t> RSAHandler::parseEncryptedString(const std::string& encrypted) {
	std::vector<int64_t> encryptedList;
	// Remove brackets and split by comma and space
	size_t startPos = encrypted.find("[") + 1;
	size_t endPos = encrypted.find("]");
	std::string numbers = encrypted.substr(startPos, endPos - startPos);
	size_t pos = 0;
	while ((pos = numbers.find(", ")) != std::string::npos) {
		std::string numberStr = numbers.substr(0, pos);
		encryptedList.push_back(std::stoll(numberStr));
		numbers.erase(0, pos + 2);
	}
	// Last number
	encryptedList.push_back(std::stoll(numbers));
	return encryptedList;
}

vector<int64_t> RSAHandler::stringToVector(string str)
{
	// String will be like this: "[1,2,3,4,5]"

	vector<int64_t> result;
	string temp = "";

	for (int i = 0; i < str.length(); i++) {
		if (str[i] == '[') {
			continue;
		}

		else if (str[i] == ',' || str[i] == ']') {
			result.push_back(std::stoll(temp));
			temp = "";
		}

		else {
			temp += str[i];
		}
	}

	return result;
}

string RSAHandler::formatForSending()
{
	return to_string(publicKey) + SPLITER + to_string(modulus);
}

int64_t RSAHandler::generate_prime(int64_t big_num) {
	int64_t prime = rand() % big_num + 1;

	while (!is_prime(prime)) {
		prime = rand() % big_num + 1;
	}

	return prime;
}

int64_t RSAHandler::totient(int64_t n) {
	int64_t result = n;

	for (int64_t i = 2; i <= sqrt(n); i++) {
		if (n % i == 0) {
			while (n % i == 0) {
				n /= i;
			}

			result -= result / i;
		}
	}

	if (n > 1) {
		result -= result / n;
	}

	return result;
}

bool RSAHandler::is_prime(int64_t n) {
	if (n <= 1) return false;

	for (int64_t i = 2; i <= sqrt(n); i++) {
		if (n % i == 0) return false;
	}

	return true;
}

int64_t RSAHandler::generate_e(int64_t num) {
	for (int64_t e = 2; e < num; e++) {
		if (gcd(e, num) == 1) {
			return e;
		}
	}

	throw std::runtime_error("Couldn't generate e");
}

int64_t RSAHandler::mod_pow(int64_t base, int64_t exponent, int64_t modulus) {
	int64_t result = 1;

	while (exponent > 0) {
		if (exponent % 2 == 1) {
			result = (result * base) % modulus;
		}
		base = (base * base) % modulus;
		exponent /= 2;
	}

	return result;
}

int64_t RSAHandler::calculate_d(int64_t e, int64_t totient) {
	int64_t d = mod_inverse(e, totient);
	return d;
}

int64_t RSAHandler::mod_inverse(int64_t a, int64_t m) {
	int64_t m0 = m;
	int64_t y = 0, x = 1;

	if (m == 1) return 0;

	while (a > 1) {
		int64_t q = a / m;
		int64_t t = m;

		m = a % m;
		a = t;
		t = y;

		y = x - q * y;
		x = t;
	}

	if (x < 0) x += m0;

	return x;
}

int64_t RSAHandler::gcd(int64_t a, int64_t b) {
	while (b != 0) {
		int64_t temp = b;
		b = a % b;
		a = temp;
	}
	return a;
}
