#include "../ServerProject/ClientConnection.h"
#include "../ServerProject/ConversationObject.h"
#include "Client.h"
#include "RelayObject.h"
#include <thread>

using std::thread;

void commandLine(Client* client, ClientConnection* entry) {
	while (true) {
		string input = "";

		std::getline(std::cin, input);

		if (input == "exit") {
			client->stopClient();
			exit(0);
		}

		try {
			DestinationData dd(input);
			client->sendData(dd, entry);
		}
		catch (...) {
			vector<string> split = Utility::splitString(input, SPLITER);
			if (split.size() == 2) {
				string username = split[0];
				string message = Utility::formatData(split[1]);

				client->sendData(username, message, entry);
			}
			else {
				cout << "Invalid input format. Please provide input in the format 'username::::message'" << endl;
			}
		}
	}
}

int main()
{
	Client client;
	cout << "Client started" << endl;

	client.printNodes();

	/*
	ClientConnection* entry = client.connectToEntryNode();
	cout << "Connected to entry node" << endl;

	client.sendData("test", "Hello from client", entry);

	thread(commandLine, &client, entry).detach();

	while (true)
	{
		string data = entry->receiveData();

		if (data.empty())
		{
			continue;
		}

		string decrypted = client.decrypt(data);

		cout << "Received: " << decrypted << endl;
	}

	delete entry;
	*/

	Sleep(1000000 * 10000 * 1000000);

	return 0;
}