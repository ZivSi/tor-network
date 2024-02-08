#include <iostream>
#include <thread>
#include "Node.h"
#include "Utility.h"

using std::thread;


int main() {
	Node node1;
	Node node2;
	Node node3;
	Node node4;
	Node node5;

	thread nodeServer(&Node::start, &node1);
	thread nodeServer1(&Node::start, &node2);
	thread nodeServer2(&Node::start, &node3);
	thread nodeServer3(&Node::start, &node4);
	thread nodeServer4(&Node::start, &node5);

	nodeServer.join();
	nodeServer1.join();
	nodeServer2.join();
	nodeServer3.join();
	nodeServer4.join();

	return 0;
}
