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

#define _CRTDBG_MAP_ALLOC //to get more details

int main()
{
	_CrtMemState sOld;
	_CrtMemState sNew;
	_CrtMemState sDiff;
	_CrtMemCheckpoint(&sOld); //take a snapshot

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
		main();
	}

	cout << "Server closed" << endl;
	_CrtMemCheckpoint(&sNew); //take a snapshot 
	if (_CrtMemDifference(&sDiff, &sOld, &sNew)) // if there is a difference
	{
		OutputDebugString(L"-----------_CrtMemDumpStatistics ---------");
		_CrtMemDumpStatistics(&sDiff);
		OutputDebugString(L"-----------_CrtMemDumpAllObjectsSince ---------");
		_CrtMemDumpAllObjectsSince(&sOld);
		OutputDebugString(L"-----------_CrtDumpMemoryLeaks ---------");
		_CrtDumpMemoryLeaks();
	}
	else {
		OutputDebugString(L"No memory leaks");
	}


	return 0;
}

