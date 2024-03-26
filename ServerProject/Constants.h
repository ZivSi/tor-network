#pragma once

#include <math.h>
#include <string>

#define RECEIVE_SIZE 2 << 18

using std::string;

namespace Constants {
	extern const string EXIT_NODE_STRING;
	extern const string LOCALHOST;
	extern const int IP_SIZE;
	extern const int PORT_SIZE;
	extern const int UUID_SIZE;
	extern const int UUID_ENCRYPTED_SIZE;
	extern const string SERVER_IP;
	extern const unsigned short SERVER_PORT;
	extern const unsigned short MAPPING_SERVER_PORT;
	extern const unsigned short LOCAL_CLIENT_PORT;

	extern const string START_PATH_DESIGN_STRING;
	extern const string ELECTRON_INIITIAL_MESSAGE;
	extern const int ELECTRON_TIMEOUT;

	extern const string SPLITER;
	extern const string NODE_INIT_FORMAT;
	extern const string CLIENT_INITIAL_MESSAGE;
	extern const int NODE_SPLIT_SIZE;
	extern const int DEFAULT_PATH_LENGTH;
	extern const int MAX_PATH_LENGTH;
	extern const int MAX_TIME_ALIVE;
	extern const int MAX_PING_ALIVE_MS;
	extern const int PATH_TIMEOUT;
	extern const int CLIENT_WAIT_FOR_NODES;

	extern const int AES_KEY_SIZE_BYTES;

	extern const string PEPPER;
	extern const string PEPPER2;
	extern const string PEPPER3;

	extern const string NO_NOEDS_REPONSE;
	extern const string NODE_HELLO;

	enum NodeIndexes {
		IP = 0,
		PORT = 1,
		ECC_PUBLIC_KEY = 2,
		CURRENT_TIME = 3,
		RANDOM_NON_PRIME_NUMBER = 4,
		RANDOM_NUMBER = 5,
		CONDITION_NUMBER = 6,
		HASH = 7,
		EXTENDED_HASH = 8
	};

	enum ErrorCodes {
		HOST_RESPONSE = 0,
		HOST_UNREACHABLE = 1,
		NODE_UNREACHABLE = 2,
		CONVERSATION_TIMEOUT = 3,
		USERNAME_NOT_FOUND = 4,
	};

	enum PacketUsernameIndexes {
		USERNAME_INDEX = 0,
		MESSAGE_INDEX = 1
	};


	enum BackToFront
	{
		MESSAGE = 0,
		INFO = 1,
		ERROR_NODES_LENGTH = 2,
		ERROR_USERNAME_NOT_FOUND = 3,
		ERROR_HOST_UNREACHABLE = 4,
		ERROR_PATH_NOT_COMPLETE = 5,
		ERROR_INVALID_ARGS = 6,
		USERNAME = 7,
		ERROR_CONNECTION_TIMEOUT = 8,
		ERROR_PATH_TIMEOUT = 9
	};
}
