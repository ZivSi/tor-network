#include <iostream>
#include <bitset>
#include <vector>
#include "RSA.hpp"

using namespace std;

int main() {
	RSA myRSA;
	RSA clientRSA;

	myRSA.setClientPublicKey(clientRSA.getPublicKey());
	myRSA.setClientModulus(clientRSA.getModulus());

	clientRSA.setClientPublicKey(myRSA.getPublicKey());
	clientRSA.setClientModulus(myRSA.getModulus());

	string message = "Hello World!";

	vector<int64_t> encryptedMessage = myRSA.encrypt(message);

	string decryptedMessage = clientRSA.decrypt(encryptedMessage);

	cout << "Message: " << message << endl;
	cout << "Encrypted Message: ";
	for (int i = 0; i < encryptedMessage.size(); i++) {
		cout << encryptedMessage[i] << " ";
	}

	cout << endl << "Decrypted Message: " << decryptedMessage << endl;


	return 0;
}
