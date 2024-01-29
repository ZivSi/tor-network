#include "Client.h"
#include <thread>

using std::thread;

int main()
{
	cout << "Client started" << endl;
	Client client;
	thread clientThread(&Client::receiveResponseFromServerInLoop, &client);
	clientThread.join();
	
	return 0;
}