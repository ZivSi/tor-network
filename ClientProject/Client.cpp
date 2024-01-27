#include "Client.h"

Client::Client() : logger("Client"), clientConnection("127.0.0.1", SERVER_PORT, logger)
{
	clientConnection.handshake();
	clientConnection.sendEncrypted("Hi. I'm a client");
    receiveResponseFromServer();
}

Client::~Client() {
    clientConnection.closeConnection();
}

void Client::receiveResponseFromServer()
{
    string received = clientConnection.receiveData();
    cout << "Received from server: " << received << endl;

    if (received.empty()) {
        return;
    }

    receivedPorts.clear();

    // Deserialize the received data
    size_t numPorts = received.size() / sizeof(unsigned short);
    receivedPorts.resize(numPorts);

    std::memcpy(receivedPorts.data(), received.data(), received.size());

    cout << "Received ports: " << endl;
    for (unsigned short port : receivedPorts) {
        cout << port << endl;
    }
}


