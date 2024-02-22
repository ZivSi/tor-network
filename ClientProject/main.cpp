#include "Client.h"
#include "ClientConnection.h"
#include "RelayObject.h"
#include <thread>

using std::thread;

int main()
{
	cout << "Client started" << endl;
	Client client;

	cout << "Waiting for nodes..." << endl;
	client.waitForNodes();

	client.receiveResponseFromServer();

	cout << "Received relays" << endl;
	client.startPathDesign();
	cout << "Path designed" << endl;

	client.handshakeWithCurrentPath();

	client.printNodes();

	ClientConnection* entry = client.connectToEntryNode();
	cout << "Connected to entry node" << endl;

	string keys = entry->receiveKeys(true);

	client.sendData("127.0.0.1", 10210, "Hello from client", entry);

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