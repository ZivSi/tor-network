#pragma once

#include <string>
#include <math.h>

#define RECEIVE_SIZE 2 << 20

using std::string;

namespace Constants {
    extern const int SERVER_PORT;
    extern const string SPLITER;
    extern const string NODE_INIT_FORMAT;
    extern const string CLIENT_INIT_FORMAT;
    extern const int DEFAULT_PATH_LENGTH;
    extern const int MAX_TIME_ALIVE;

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
}
