#include "ConversationObject.h"
#include "Server.h"
#include <iostream>
#include <string>
#include <thread>
#include <vector>

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
	catch (Exception e) {
		cout << e.what() << endl;
		server.stopServer();
	}

	cout << "Server closed" << endl;

	return 0;
}
