#include "Constants.h"
#include "Utility.h"
#include <format>
#include <string>


namespace Constants {
	// extern const string EXIT_NODE_STRING;
	const string EXIT_NODE_STRING = "EXIT_NODE";
	const string LOCALHOST = "127.0.0.1";
	const int IP_SIZE = static_cast<const int>(string("XXX.XXX.XXX.XXX").size());
	const int PORT_SIZE = 5; // 65535
	const int UUID_SIZE = 4; // bytes
	const int UUID_ENCRYPTED_SIZE = 89;
	const string SERVER_IP = "10.0.0.40";
	const unsigned short SERVER_PORT = 5060;
	const string SPLITER = "::::";

	// Server port::::publicKey::::Current time in ms::::Random non prime number::::Random number::::Condition number::::Hash of the first parts::::Extended hash of the rest
	const string NODE_INIT_FORMAT = "{ip}" + SPLITER + "{port}" + SPLITER + "{publickey}" + SPLITER + "{time}" + SPLITER + "{nonprime}" + SPLITER + "{randmomNumber}" + SPLITER + "{modulus base}" + SPLITER + "{hash}" + SPLITER + "{ext_hash}";


	const int NODE_SPLIT_SIZE = static_cast<const int>(Utility::splitString(NODE_INIT_FORMAT, SPLITER).size());
	const int DEFAULT_PATH_LENGTH = 3;
	const int MAX_TIME_ALIVE = 10000;
	const int MAX_PING_ALIVE_MS = 2000;
	const int CONVERSATION_TIMEOUT = 10 * 60 * 1000; // 10 minutes

	const int AES_KEY_SIZE_BYTES = 32;

	const string PEPPER = "myCoolPepper";
	const string PEPPER2 = "anotherPepper:)";

	const string NO_NOEDS_REPONSE = "NO_NODES";
}