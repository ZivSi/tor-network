#include "Node.h"
#include "Utility.h"
#include <iostream>
#include <thread>

using std::thread;


// We get parent ip and port from the command line
int main(int argc, char* argv[]) {
	if (argc != 3) {
		std::cerr << "Usage: " << argv[0] << " <parent ip> <parent port>" << std::endl;
		return 1;
	}

	// Get parent ip and port from the command line
	std::string parentIp = argv[1];
	unsigned short parentPort = 0;

	try {
		parentPort = std::stoi(argv[2]);
	}
	catch (std::invalid_argument& e) {
		std::cerr << "Invalid port number" << std::endl;
		return 1;
	}

	cout << "Args are - ip: " << parentIp << " port: " << parentPort << endl;

	// Create a node
	Node node(parentIp, parentPort);

	// Start the node in thread
	thread(&Node::start, &node).join();

	return 0;
}
