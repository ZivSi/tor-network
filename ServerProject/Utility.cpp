#include "Utility.h"


constexpr unsigned long long Utility::capture_time() {
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

constexpr long long Utility::calculateDifference(long long start_time) {
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
	if (input.length() < 16) {
		cerr << "Input is too short to extract key" << endl;
		return;
	}

	for (int i = 0; i < 16; i++) {
		output += input[i];
	}
}

void Utility::extractAESIv(const string& input, string& output) {
	if (input.length() < 32) {
		cerr << "Input is too short to extract IV" << endl;
		return;
	}

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

constexpr bool Utility::isPrime(unsigned long int n) {
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
	// received = UUID + IP + PORT
	// return received.substr(0, Constants::UUID_SIZE); // TODO: Replace with constant

	string idEncrypted = "";

	for (int i = 0; i < Constants::UUID_ENCRYPTED_SIZE; i++) {
		idEncrypted += received[i];
	}

	return idEncrypted;
}

string Utility::extractIpAddress(const string& received)
{
	return received.substr(Constants::UUID_ENCRYPTED_SIZE, Constants::IP_SIZE);
}

unsigned short Utility::extractPort(const string& received)
{
	return static_cast<unsigned short>(std::stoi(received.substr(Constants::UUID_ENCRYPTED_SIZE + Constants::IP_SIZE, Constants::PORT_SIZE)));
}

// The string might be big, so it will be more optimal to pass by reference
string Utility::extractData(const string& received)
{
	return received.substr(Constants::UUID_ENCRYPTED_SIZE + Constants::IP_SIZE + Constants::PORT_SIZE);
}

bool Utility::isValidIpv4(const std::string& ip)
{
	std::vector<std::string> parts;
	std::istringstream iss(ip);
	std::string part;

	// Split the string by periods
	while (getline(iss, part, '.')) {
		parts.push_back(part);
	}

	// Check for exactly 4 parts
	if (parts.size() != 4) {
		return false;
	}

	// Check each part
	for (const std::string& p : parts) {
		// Check if the part is a valid integer
		if (!isValidInteger(p)) {
			return false;
		}

		// Convert the part to an integer
		int num = stoi(p);

		// Check if the integer is in range [0, 255]
		if (num < 0 || num > 255) {
			return false;
		}

		// Check for leading zeros
		if (p.size() > 1 && p[0] == '0') {
			return false;
		}
	}

	return true;
}

bool Utility::isValidPort(unsigned short port)
{
	return (port > 0 && port <= 65535);
}

bool Utility::isValidInteger(const std::string& str)
{
	if (str.empty()) {
		return false;
	}

	// Check if each character is a digit
	for (char c : str) {
		if (!std::isdigit(c)) {
			return false;
		}
	}

	return true;
}