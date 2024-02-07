#include "Utility.h"


unsigned long long Utility::capture_time() {
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

long long Utility::calculate_time(long long start_time) {
	return capture_time() - start_time;
}

vector<string> Utility::splitString(const string& input, const string& delimiter) {
	vector<string> tokens;
	size_t start = 0, end = 0;
	while ((end = input.find(delimiter, start)) != string::npos) {
		tokens.push_back(input.substr(start, end - start));
		start = end + delimiter.length();
	}
	tokens.push_back(input.substr(start));
	return tokens;
}

void Utility::extractAESKey(const string& input, string& output) {
	for (int i = 0; i < 16; i++) {
		output += input[i];
	}
}

void Utility::extractAESIv(const string& input, string& output) {
	for (int i = 16; i < 32; i++) {
		output += input[i];
	}
}

bool Utility::isNumber(const string& input) {
	for (int i = 0; i < input.length(); i++) {
		if (!isdigit(input[i])) {
			return false;
		}
	}
	return true;
}

bool Utility::isValidECCPublicKey(const string& input) {
	return true;
}

bool Utility::isValidTime(const string& input) {
	if (!isNumber(input)) {
		return false;
	}
	return capture_time() - std::stoll(input) < Constants::MAX_PING_ALIVE_MS;
}

bool Utility::isPrime(unsigned long int n) {
	if (n < 2) {
		return false;
	}
	for (unsigned long int i = 2; i <= sqrt(n); i++) {
		if (n % i == 0) {
			return false;
		}
	}
	return true;
}

unsigned long int Utility::findModuloBase(unsigned long int nonPrime, unsigned short port) {
	for (unsigned long int condition = port + 1; condition < nonPrime / 2; condition++) {
		if (nonPrime % (condition - port) == 1) {
			return condition;
		}
	}

	cerr << "Couldn't find condition number (panic now)" << endl;
	return 0;
}

string Utility::asHex(string input) // Represent every string by a printable string
{
	std::stringstream hexStringStream;

	for (int i = 0; i < 16; i++) {
		unsigned char ch = input.at(i);
		hexStringStream << std::hex << std::setw(2) << std::setfill('0') << (int)ch;
	}

	return hexStringStream.str();
}

string Utility::hashStr(const string& input) {
	SHA256 sha256;
	string hashed;

	StringSource(input, true, new CryptoPP::HashFilter(sha256, new CryptoPP::HexEncoder(new StringSink(hashed))));

	return hashed;
}

unsigned long int Utility::generateNonPrime() {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<unsigned long int> dis(2000000, 10000000);

	unsigned long int n = 0;
	do {
		n = dis(gen);
	} while (Utility::isPrime(n));

	return n;
}

unsigned long int Utility::generateRandomNumber(unsigned long int lowerLimit, unsigned long int upperLimit) {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<unsigned long int> dis(lowerLimit, upperLimit);

	return dis(gen);
}

string Utility::extractConversationId(const string& received)
{
	return received.substr(0, 4); // TODO: Replace with constant
}
