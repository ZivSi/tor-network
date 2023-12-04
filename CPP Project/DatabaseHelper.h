#pragma once

#include "sqlite3.h"
#include "Client.hpp"
#include "Node.hpp"
#include "Log.hpp"


class DatabaseHelper
{
public:
	DatabaseHelper();
	~DatabaseHelper();

	// Db methods
	bool openDatabase();
	bool closeDatabase();
	void initializeDatabase();
	bool isTableExist(string table_name);


	bool isClientExist(string username);
	bool isPasswordCorrect(string username, string password);
	ClientData getClientByUsername(string username);
	bool insertClient(ClientData client_data);
	bool updateClient(ClientData client_data);
	bool deleteClient(string username);

	bool isNodeExist(string server_port);
	NodeData getNodeByServerPort(string server_port);
	bool insertNode(NodeData node_data);
	bool updateNode(NodeData node_data);
	bool deleteNode(string server_port);

private:
	sqlite3* m_database;

	string filename = "tor.db";

	string tables[3] = { "clients", "nodes", "stats" };
	string clients_columns[7] = { "username", "password", "email", "bytes_sent", "bytes_received", "aes_key", "aes_iv" };

	string nodes_columns[5] = { "server_port", "time", "connection", "aes_key", "aes_iv" };

	string stats_columns[6] = { "total_bytes_sent", "total_bytes_received", "total_connections", "total_nodes", "total_clients", "total_stats" };
};
