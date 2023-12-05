#include <cstdlib>
#include <cmath>
#include <vector>
#include <stdexcept>
#include <cstdint>
#include <numeric>
#include <string>

using std::string;

int64_t gcd(int64_t a, int64_t b) {
	while (b != 0) {
		int64_t temp = b;
		b = a % b;
		a = temp;
	}

	return a;
}

class RSA {
public:
	RSA(int key_size = 512) {
		int64_t biggest_num = pow(2, key_size - 1);

		p = generate_prime(biggest_num);
		q = generate_prime(biggest_num);

		modulus = p * q;
		t = totient(modulus);

		publicKey = generate_e(t);
		privateKey = calculate_d(publicKey, t);
	}

	void setClientPublicKey(int64_t clientPublicKey) {
		this->clientPublicKey = clientPublicKey;
	}

	void setClientModulus(int64_t clientModulus) {
		this->clientModulus = clientModulus;
	}

	int64_t getPublicKey() {
		return publicKey;
	}

	int64_t getModulus() {
		return modulus;
	}

	int64_t getPrivateKey() {
		return privateKey;
	}


	std::vector<int64_t> encrypt(string m) {
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

	std::string decrypt(const std::vector<int64_t>& encrypted) {
		std::string decryptedMessage;

		for (int64_t encryptedChar : encrypted) {
			char decryptedChar = static_cast<char>(mod_pow(encryptedChar, privateKey, modulus));
			decryptedMessage += decryptedChar;
		}

		return decryptedMessage;
	}

private:
	int64_t generate_prime(int64_t big_num) {
		int64_t prime = rand() % big_num + 1;

		while (!is_prime(prime)) {
			prime = rand() % big_num + 1;
		}

		return prime;
	}

	int64_t totient(int64_t n) {
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

	bool is_prime(int64_t n) {
		if (n <= 1) return false;

		for (int64_t i = 2; i <= sqrt(n); i++) {
			if (n % i == 0) return false;
		}

		return true;
	}

	int64_t generate_e(int64_t num) {
		for (int64_t e = 2; e < num; e++) {
			if (gcd(e, num) == 1) {
				return e;
			}
		}

		throw std::runtime_error("Couldn't generate e");
	}

	int64_t mod_pow(int64_t base, int64_t exponent, int64_t modulus) {
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

	int64_t calculate_d(int64_t e, int64_t totient) {
		// Using the extended Euclidean algorithm to find the modular multiplicative inverse
		int64_t d = mod_inverse(e, totient);
		return d;
	}

	int64_t mod_inverse(int64_t a, int64_t m) {
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

	int64_t p;
	int64_t q;
	int64_t modulus;
	int64_t t;

	int64_t publicKey;
	int64_t privateKey;

	int64_t clientPublicKey = 0;
	int64_t clientModulus = 0;
};
