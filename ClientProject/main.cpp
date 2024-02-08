#include "Client.h"
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
	
	return 0;
}