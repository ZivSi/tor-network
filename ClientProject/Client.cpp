#include "Client.h"

Client::Client() : logger("Client"), clientConnection("127.0.0.1", SERVER_PORT, logger)
{
	clientConnection.handshake();
	clientConnection.sendEncrypted("Hi. I'm a client"); // Initial message
}

Client::~Client() {
    clientConnection.closeConnection();
}

void Client::receiveResponseFromServer()
{
    string received = clientConnection.receiveData();
    cout << "Received from server: " << received << endl;

    if (received.empty() || received == NO_NOEDS_REPONSE) {
        logger.error("No nodes available");

        return;
    }

    receivedPorts.clear();

    // Deserialize the received data
    size_t numPorts = received.size() / sizeof(unsigned short);
    receivedPorts.resize(numPorts);

    std::memcpy(receivedPorts.data(), received.data(), received.size());

    cout << "Received ports: [";
    for (unsigned short port : receivedPorts) {
        cout << port << ", ";
    }

    // Delete last comma
    cout << "\b\b]" << endl;
}

void Client::receiveResponseFromServerInLoop()
{
    while (true) {
        receiveResponseFromServer();

        clientConnection.closeConnection();

        Sleep(3000);

        clientConnection.connectInLoop();
        clientConnection.handshake();
        clientConnection.sendEncrypted("Hi. I'm a client");
    }
}


