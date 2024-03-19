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
				string message = formatData(split[1]);

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

	ClientConnection* entry = client.connectToEntryNode();
	cout << "Connected to entry node" << endl;

	string keys = entry->receiveKeys(true);

	client.sendData("test", "Hello from client", entry);

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