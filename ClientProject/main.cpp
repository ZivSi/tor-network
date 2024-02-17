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
	string keys = entry->receiveKeys(true);

	client.sendData(SERVER_IP, 6969, "Hello from client", entry);

	delete entry;


	return 0;
}