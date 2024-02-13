#include "Client.h"
#include "ClientConnection.h"
#include "RelayObject.h"
#include <thread>

using std::thread;

int main()
{
	cout << "Client started" << endl;
	Client client;

	client.waitForNodes();

	client.receiveResponseFromServer();
	client.startPathDesign();
	client.handshakeWithCurrentPath();

	client.printNodes();

	ClientConnection* entry = client.connectToEntryNode();
	string keys = entry->receiveKeys(true);

	cout << "Keys: " << keys << endl;
	client.sendData("127.0.0.1", 6969, "Hello from client", entry);

	delete entry;


	return 0;
}