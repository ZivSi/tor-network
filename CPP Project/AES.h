#include <iostream>
#include <string>


class AES {
public:
    AES();  // Constructor
    std::string encrypt(const std::string& plaintext);
    std::string decrypt(const std::string& ciphertext);

private:
    unsigned char key[16];
    unsigned char iv[16];
};

std::string encrypt(const std::string& plaintext, const unsigned char* key, const unsigned char* iv);
std::string decrypt(const std::string& ciphertext, const unsigned char* key);
std::string format_key_for_sending(const unsigned char* key);
std::string reformat_key_for_receiving(const std::string& key);
