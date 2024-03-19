#include "Constants.h"
#include "Utility.h"
#include <format>
#include <string>


namespace Constants {
	const string EXIT_NODE_STRING = "EXIT_NODE";
	const string LOCALHOST = "127.0.0.1";
	const int IP_SIZE = static_cast<const int>(string("XXX.XXX.XXX.XXX").size());
	const int PORT_SIZE = 5; // 2 ** 16 = 65535 -> "65535" 5 char length
	const int UUID_SIZE = 4; // bytes
	const int UUID_ENCRYPTED_SIZE = 89;
	const string SERVER_IP = "127.0.0.1";
	const unsigned short SERVER_PORT = 5060;
	const unsigned short MAPPING_SERVER_PORT = 5060 * 2;
	const unsigned short LOCAL_CLIENT_PORT = 5060 * 3;
	// START_PATH_DESIGN_STRING
	const string START_PATH_DESIGN_STRING = "START PATH DESIGN";
	const string SPLITER = "::::";

	// Server port::::publicKey::::Current time in ms::::Random non prime number::::Random number::::Condition number::::Hash of the first parts::::Extended hash of the rest
	const string NODE_INIT_FORMAT = "{ip}" + SPLITER + "{port}" + SPLITER + "{publickey}" + SPLITER + "{time}" + SPLITER + "{nonprime}" + SPLITER + "{randmomNumber}" + SPLITER + "{modulus base}" + SPLITER + "{hash}" + SPLITER + "{ext_hash}";

	const string CLIENT_INITIAL_MESSAGE = "Hi. I'm a client";

	const int NODE_SPLIT_SIZE = static_cast<const int>(Utility::splitString(NODE_INIT_FORMAT, SPLITER).size());
	const int DEFAULT_PATH_LENGTH = 3;
	const int MAX_TIME_ALIVE = 10000;
	const int MAX_PING_ALIVE_MS = 2000;
	const int PATH_TIMEOUT = 10 * 60 * 1000; // 10 minutes
	const int CLIENT_WAIT_FOR_NODES = 3000;

	const int AES_KEY_SIZE_BYTES = 32;

	const string PEPPER = "myCoolPepper";
	const string PEPPER2 = "anotherPepper:)";
	const string PEPPER3 = "thisIsAPepperForTheKotlinServerYe=s";

	const string NO_NOEDS_REPONSE = "NO_NODES";
	const string NODE_HELLO = "HELLO";
}
