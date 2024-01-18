#include "Constants.h"
#include "Utility.h"
#include <format>


namespace Constants {
	const unsigned short SERVER_PORT = 5060;
	const string SPLITER = "::::";

	// Server port::::publicKey::::Current time in ms::::Random non prime number::::Random number::::Condition number::::Hash of the first parts::::Extended hash of the rest
	const string NODE_INIT_FORMAT = "{}" + SPLITER + "{}" + SPLITER + "{}" + SPLITER + "{}" + SPLITER + "{}" + SPLITER + "{}" + SPLITER + "{}" + SPLITER + "{}";
	const string CLIENT_INIT_FORMAT = "{}" + SPLITER + "{}";

	const int NODE_SPLIT_SIZE = Utility::splitString(NODE_INIT_FORMAT, SPLITER).size();
	const int DEFAULT_PATH_LENGTH = 5;
	const int MAX_TIME_ALIVE = 10000;
	const int MAX_PING_ALIVE_MS = 2000;

	const int AES_KEY_SIZE_BYTES = 32;

	const string PEPPER = "myCoolPepper";
	const string PEPPER2 = "anotherPepper:)";
}
