#pragma once

#include "Constants.h"
#include "Logger.h"
#include "NodeData.h"
#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <secblock.h>
#include <sstream>
#include <string>
#include <vector>

typedef unsigned char byte;

using std::string;
using std::vector;
using CryptoPP::SecByteBlock;

class Utility {
public:
	// ----------------- Time Functions -----------------
	static constexpr unsigned long long capture_time();
	static constexpr long long calculateDifference(long long start_time);

	// ----------------- String Manipulation -----------------
	static vector<string> splitString(const string& input, const string& delimiter = Constants::SPLITER);
	static void extractAESKey(const string& input, string& output);
	static void extractAESIv(const string& input, string& output);
	static bool isNumber(const string& input);
	static bool isValidECCPublicKey(const string& input);
	static bool isValidTime(const string& input);
	static constexpr bool isPrime(unsigned long int n);
	static string asHex(string input);
	static string hashStr(const string& input);

	// ----------------- Random Number Generation -----------------
	static unsigned long int generateNonPrime();
	static unsigned long int generateRandomNumber(unsigned long int lowerLimit, unsigned long int upperLimit);

	static unsigned long int findModuloBase(unsigned long int nonPrime, unsigned short port);

	// ----------------- Data Extraction -----------------
	static string extractConversationId(const string& received);
	static string extractIpAddress(const string& received);
	static unsigned short extractPort(const string& received);
	static string extractData(const string& received);

	static string formatData(const string& data);


	// ----------------- Validation -----------------
	static bool isValidIpv4(const std::string& ip);
	static bool isValidPort(unsigned short port);
	static bool isValidInteger(const std::string& str);
};
