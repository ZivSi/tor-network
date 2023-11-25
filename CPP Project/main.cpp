#include <iostream>
#include "Log.hpp"
#include "Utility.hpp"
#include "Constants.h"

using std::cout;
using std::endl;

int main()
{
    NodeData node_data = NodeData(5061, "2021-05-01 12:00:00", "", "aes_key", "aes_iv");
    cout << node_data.server_port << endl;

    // Clone check

    return 0;
}