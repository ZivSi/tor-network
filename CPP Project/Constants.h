#include <string>
#include <math.h>

using std::string;

int SERVER_PORT = 5060;
double RECEIVE_SIZE = pow(2, 24);
string SPLITER = "::::";
string NODE_INIT_FORMAT = "{}" + SPLITER + "{}" + SPLITER + "{}";
string CLIENT_INIT_FORMAT = "{}" + SPLITER + "{}";
int DEFAULT_LEN = 5;

enum NodeIndexes {
	NODE_SERVER_PORT_INDEX = 0,
	NODE_TIME_INDEX = 1,
	NODE_CONNECTION_INDEX = 2,
	NODE_AES_KEY_INDEX = 3,
	NODE_AES_IV_INDEX = 4
};

enum ClientIndexes {
	CONNECTION_INDEX = 0,
	TIME_INDEX = 1,
	AES_KEY_INDEX = 2,
	AES_IV_INDEX = 3
};
