#include <iostream>
#include <bitset>
#include <vector>
#include "AesHandler.h"

using namespace std;

int main() {
	AesHandler aes_handler;
	string plaintext = "Hello World!";


	string ciphertext = aes_handler.encrypt(plaintext);
	cout << "Ciphertext: " << ciphertext << endl;

	string decrypted = aes_handler.decrypt(ciphertext);
	cout << "Decrypted: " << decrypted << endl;


	return 0;
}
