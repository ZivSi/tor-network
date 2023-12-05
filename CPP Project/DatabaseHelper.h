#pragma once

#include "sqlite3.h"
#include "NodeData.hpp"
#include <iostream>
#include <string>

using std::cout;
using std::endl;
using std::string;
using std::to_string;

class DatabaseHelper
{
public:
	DatabaseHelper();
	~DatabaseHelper();

	void executeSimpleQuery(const std::string& query);
	int executeQueryWithCallback(const std::string& query, sqlite3_callback callback, void* callbackData);

	bool isDatabaseOpen();
	bool openDatabase();
	bool closeDatabase();
	void initializeDatabase();

	bool isTableExist(string table_name);

	void increaseTotalBytesSent(long long int bytes);
	void increaseTotalBytesReceived(long long int bytes);
	void increaseTotalConnections();
	void increaseTotalNodes();
	void increaseTotalClients();

	void resetStats();
	void addOneEntryToStatsTable();
	void printStats();


	long long int getTotalBytesSent();
	long long int getTotalBytesReceived();
	int getTotalConnections();
	int getTotalNodes();
	int getTotalClients();

private:
	sqlite3* m_database;

	string filename = "tor.db";

	string tables[1] = { "stats" };

	string stats_columns[6] = { "total_bytes_sent", "total_bytes_received", "total_connections", "total_nodes", "total_clients" };
};
