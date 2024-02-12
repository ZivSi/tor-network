#include <iostream>
#include "Server.h"
#include <thread>
#include <string>
#include <vector>
#include "ConversationObject.h"

using std::vector;
using std::cout;
using std::endl;
using std::thread;
using std::string;
using std::to_string;

int main()
{
    Server server;
    try {
        thread serverThread(&Server::startServer, &server);

        serverThread.detach();

        while (true) {
            string input;
            std::getline(std::cin, input);

            if (input == "exit") {
                server.stopServer();
                break;
            }
        }
    }
    catch (Exception) {
        server.stopServer();
    }

    cout << "Server closed" << endl;

    return 0;
}
