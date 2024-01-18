#pragma once

#include <string>
#include <math.h>

#define RECEIVE_SIZE 2 << 18

using std::string;

namespace Constants {
	extern const unsigned short SERVER_PORT;
	extern const string SPLITER;
	extern const string NODE_INIT_FORMAT;
	extern const int NODE_SPLIT_SIZE;
	extern const string CLIENT_INIT_FORMAT;
	extern const int DEFAULT_PATH_LENGTH;
	extern const int MAX_TIME_ALIVE;
	extern const int MAX_PING_ALIVE_MS;

	extern const int AES_KEY_SIZE_BYTES;

	extern const string PEPPER;
	extern const string PEPPER2;

	// Port||||PublicKeyAsString||||CurrentTime||||randomNonPrimeNumber||||number that make(random % (number – port) == 0
	enum NodeIndexes {
		PORT = 0,
		ECC_PUBLIC_KEY = 1,
		CURRENT_TIME = 2,
		RANDOM_NON_PRIME_NUMBER = 3,
		RANDOM_NUMBER = 4,
		CONDITION_NUMBER = 5,
		HASH = 6,
		EXTENDED_HASH = 7
	};
}
