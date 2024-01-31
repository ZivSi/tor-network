#pragma once

#include "Constants.h"
#include "Logger.h"
#include "NodeData.h"
#include <chrono>
#include <iostream>
#include <random>
#include <secblock.h>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>
#include "ConversationObject.h"

typedef unsigned char byte;

using std::string;
using std::vector;
using CryptoPP::SecByteBlock;



class Utility {
public:
	static unsigned long long capture_time();
	static long long calculate_time(long long start_time);
	static void print_path();
	static vector<string> splitString(const string& input, const string& delimiter = Constants::SPLITER);
	static void extractAESKey(const string& input, string& output);
	static void extractAESIv(const string& input, string& output);
	static bool isNumber(const string& input);
	static bool isValidECCPublicKey(const string& input);
	static bool isValidTime(const string& input);
	static bool isPrime(unsigned long int n);
	static unsigned long int findModuloBase(unsigned long int n, unsigned short port);
	static string asHex(string input);

	static string hashStr(const string& input);
	static unsigned long int generateNonPrime();
	static unsigned long int generateRandomNumber(unsigned long int lowerLimit, unsigned long int upperLimit);

	static string extractConversationId(const string& received);
};
