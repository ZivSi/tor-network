#include "../ServerProject/ClientConnection.h"
#include "../ServerProject/ConversationObject.h"
#include "Client.h"
#include "RelayObject.h"
#include <thread>

using std::thread;

string formatData(const string& data) {
	string formattedData = data;

	// Iterate through the string and replace occurrences of "\\n" with "\n"
	size_t pos = formattedData.find("\\n");
	while (pos != string::npos) {
		formattedData.replace(pos, 2, "\n"); // Replace 2 characters with a single newline character
		pos = formattedData.find("\\n", pos + 1); // Find next occurrence starting from pos + 1
	}

	return formattedData;
}

void commandLine(Client* client, ClientConnection* entry) {
	while (true) {
		// Get input from the user
		string input;
		std::getline(std::cin, input);

		if (input == "exit") {
			client->stopClient();

			exit(0);
		}

		try {
			DestinationData dd(input);

			client->sendData(dd.getDestinationIP(), dd.getDestinationPort(), formatData(dd.getData()), entry);
		}
		catch (...) {
			cerr << "Invalid input" << endl;
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
	}

	delete entry;

	return 0;
}