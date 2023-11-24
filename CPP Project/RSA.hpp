#include <cstdlib>
#include <cmath>
#include <vector>
#include <stdexcept>

class RSA {
public:
    RSA(int key_size = 2048);

    void setClientPublicKey(int clientPublicKey);
    void setClientModulus(int clientModulus);

    std::vector<int> encrypt(const std::string& m);
    std::string decrypt(const std::vector<int>& encrypted);

private:
    int generate_prime(int big_num);
    int totient(int n);
    bool is_prime(int n);
    int generate_e(int num);
    int calculate_d(int e, int totient);

    int p;
    int q;
    int modulus;
    int t;
    int publicKey;
    int privateKey;

    int clientPublicKey;
    int clientModulus;
};

std::vector<int> encrypt(const std::string& m, int clientPublicKey, int clientModulus);

int main();
