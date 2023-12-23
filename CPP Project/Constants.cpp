#include "Constants.h"
#include <format>


namespace Constants {
	const int SERVER_PORT = 5060;
	const string SPLITER = "::::";

	// Server port |||| aes_key |||| aes_iv
	const string NODE_INIT_FORMAT = "{}" + SPLITER + "{}" + SPLITER + "{}";
	const string CLIENT_INIT_FORMAT = "{}" + SPLITER + "{}";
	const int DEFAULT_PATH_LENGTH = 5;
	const int MAX_TIME_ALIVE = 10;
}
