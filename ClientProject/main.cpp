#include "../ServerProject/ClientConnection.h"
#include "../ServerProject/ConversationObject.h"
#include "Client.h"
#include "RelayObject.h"
#include <thread>

using std::thread;

void commandLine(Client* client, ClientConnection* entry) {
	while (true) {
		cout << "Enter input: ";

		// Get input from the user
		string input;
		std::getline(std::cin, input);

		if (input == "exit") {
			client->stopClient();

			exit(0);
		}

		try {
			DestinationData dd(input);

			client->sendData(dd.getDestinationIP(), dd.getDestinationPort(), dd.getData(), entry);
		}
		catch (...) {
			exit(1);
		}
	}
}

int main()
{
	Client client;
	cout << "Client started" << endl;

	client.printNodes();

	ClientConnection* entry = client.connectToEntryNode();
	cout << "Connected to entry node" << endl;

	string keys = entry->receiveKeys(true);

	client.sendData("127.0.0.1", 10210, "Hello from client", entry);

	thread(commandLine, &client, entry).detach();

	while (true)
	{
		string data = entry->receiveData();

		if (data == "")
		{
			continue;
		}

		string decrypted = client.decrypt(data);

		cout << "Received: " << decrypted << endl;

		Sleep(3000);
	}

	delete entry;

	return 0;
}