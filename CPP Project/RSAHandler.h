#pragma once

#pragma warning(disable : 4244)

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

using std::string;

class RSAHandler {
public:
    RSAHandler(int key_size = 512);

    void setClientPublicKey(int64_t clientPublicKey);
    void setClientModulus(int64_t clientModulus);

    int64_t getPublicKey();
    int64_t getModulus();
    int64_t getPrivateKey();

    std::vector<int64_t> encrypt(string m);
    std::string decrypt(const std::vector<int64_t>& encrypted);

private:
    int64_t generate_prime(int64_t big_num);
    int64_t totient(int64_t n);
    bool is_prime(int64_t n);
    int64_t generate_e(int64_t num);
    int64_t mod_pow(int64_t base, int64_t exponent, int64_t modulus);
    int64_t calculate_d(int64_t e, int64_t totient);
    int64_t mod_inverse(int64_t a, int64_t m);

    int64_t gcd(int64_t a, int64_t b);

private:
    int64_t p;
    int64_t q;
    int64_t modulus;
    int64_t t;

    int64_t publicKey;
    int64_t privateKey;

    int64_t clientPublicKey = 0;
    int64_t clientModulus = 0;
};
