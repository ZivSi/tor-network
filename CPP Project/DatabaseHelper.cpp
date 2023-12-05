#include "DatabaseHelper.h"


DatabaseHelper::DatabaseHelper()
{
	initializeDatabase();

	openDatabase();
}

DatabaseHelper::~DatabaseHelper()
{
	closeDatabase();
}

bool DatabaseHelper::isDatabaseOpen()
{
	return m_database != nullptr;
}

bool DatabaseHelper::openDatabase()
{
	int rc = sqlite3_open(filename.c_str(), &m_database);

	if (rc)
	{
		sqlite3_errmsg(m_database);

		return false;
	}
	else
	{
		// cout << "Opened databse successfully" << endl;
		return true;
	}
}

bool DatabaseHelper::closeDatabase()
{
	sqlite3_close(m_database);
	return true;
}

void DatabaseHelper::initializeDatabase()
{
	openDatabase();

	char* error_message = nullptr;

	if (isTableExist("stats")) {
		closeDatabase();

		cout << "Table stats already exists\n" << endl;

		return;
	}

	string create_table_query = "CREATE TABLE IF NOT EXISTS stats (total_bytes_sent INTEGER, total_bytes_received INTEGER, total_connections INTEGER, total_nodes INTEGER, total_clients INTEGER);";

	int rc = sqlite3_exec(m_database, create_table_query.c_str(), NULL, 0, &error_message);

	if (rc != SQLITE_OK)
	{
		cout << error_message << endl;

		sqlite3_free(error_message);
	}
	else
	{
		cout << "Table stats created successfully\n" << endl;
	}

	addOneEntryToStatsTable();

	closeDatabase();
}

void DatabaseHelper::addOneEntryToStatsTable()
{
	openDatabase();

	char* error_message = nullptr;

	string query = "INSERT INTO stats (total_bytes_sent, total_bytes_received, total_connections, total_nodes, total_clients) VALUES (0, 0, 0, 0, 0);";
	int rc = sqlite3_exec(m_database, query.c_str(), NULL, 0, &error_message);

	if (rc != SQLITE_OK)
	{
		if (error_message != nullptr)
		{
			cout << error_message << endl;
			sqlite3_free(error_message);
		}
		else
		{
			cout << "add entry: Error message is null" << endl;
		}
	}
	else
	{
		cout << "One entry added to stats table successfully\n" << endl;
	}

	closeDatabase();
}

bool DatabaseHelper::isTableExist(string table_name)
{
	string query = "select count(type) from sqlite_master where type='table' and name='" + table_name + "';";

	sqlite3_stmt* stmt;

	int result = sqlite3_prepare_v2(m_database, query.c_str(), -1, &stmt, nullptr);

	if (result != SQLITE_OK)
	{
		throw std::exception("Failed to execute query.");
	}

	int count = 0;

	if (sqlite3_step(stmt) == SQLITE_ROW)
	{
		count = sqlite3_column_int(stmt, 0);
	}

	sqlite3_finalize(stmt);

	return count > 0;
}




void DatabaseHelper::executeSimpleQuery(const std::string& query)
{
	int result = sqlite3_exec(m_database, query.c_str(), nullptr, nullptr, nullptr);
	if (result != SQLITE_OK)
	{
		std::string errorMessage = "Failed to execute query: " + query;
		throw std::exception(errorMessage.c_str());
	}
}

int DatabaseHelper::executeQueryWithCallback(const std::string& query, sqlite3_callback callback, void* callbackData)
{
	char* errorMessage = nullptr;
	int result = sqlite3_exec(m_database, query.c_str(), callback, callbackData, &errorMessage);
	if (result != SQLITE_OK)
	{
		std::string errorMessageStr = "Failed to execute query: " + query;
		if (errorMessage != nullptr)
		{
			errorMessageStr += " Error: ";
			errorMessageStr += errorMessage;
		}
		sqlite3_free(errorMessage);
		throw std::exception(errorMessageStr.c_str());
	}
	return result;
}


void DatabaseHelper::increaseTotalBytesSent(long long int bytes)
{
	openDatabase();

	char* error_message = nullptr;

	string query = "UPDATE stats SET total_bytes_sent = total_bytes_sent + " + to_string(bytes) + ";";
	int rc = sqlite3_exec(m_database, query.c_str(), NULL, 0, &error_message);

	if (rc != SQLITE_OK)
	{
		if (error_message != nullptr)
		{
			cout << error_message << endl;
			sqlite3_free(error_message);
		}
		else
		{
			cout << "Error message is null" << endl;
		}
	}
	else
	{
		cout << "Total bytes sent updated successfully\n" << endl;
	}

	closeDatabase();
}

void DatabaseHelper::increaseTotalBytesReceived(long long int bytes)
{
	openDatabase();

	char* error_message = nullptr;

	string query = "UPDATE stats SET total_bytes_received = total_bytes_received + " + to_string(bytes) + ";";
	int rc = sqlite3_exec(m_database, query.c_str(), NULL, 0, &error_message);

	if (rc != SQLITE_OK)
	{
		cout << error_message << endl;
		sqlite3_free(error_message);
	}
	else
	{
		cout << "Total bytes received updated successfully\n" << endl;
	}

	closeDatabase();
}

void DatabaseHelper::increaseTotalConnections()
{
	openDatabase();

	char* error_message = nullptr;

	string query = "UPDATE stats SET total_connections = total_connections + 1;";
	int rc = sqlite3_exec(m_database, query.c_str(), NULL, 0, &error_message);

	if (rc != SQLITE_OK)
	{
		cout << error_message << endl;
		sqlite3_free(error_message);
	}
	else
	{
		cout << "Total connections updated successfully\n" << endl;
	}

	closeDatabase();
}

void DatabaseHelper::increaseTotalNodes()
{
	openDatabase();

	char* error_message = nullptr;

	string query = "UPDATE stats SET total_nodes = total_nodes + 1;";
	int rc = sqlite3_exec(m_database, query.c_str(), NULL, 0, &error_message);

	if (rc != SQLITE_OK)
	{
		cout << error_message << endl;
		sqlite3_free(error_message);
	}
	else
	{
		cout << "Total nodes updated successfully\n" << endl;
	}

	closeDatabase();
}

void DatabaseHelper::increaseTotalClients()
{
	openDatabase();

	char* error_message = nullptr;

	string query = "UPDATE stats SET total_clients = total_clients + 1;";
	int rc = sqlite3_exec(m_database, query.c_str(), NULL, 0, &error_message);

	if (rc != SQLITE_OK)
	{
		cout << error_message << endl;
		sqlite3_free(error_message);
	}
	else
	{
		cout << "Total clients updated successfully\n" << endl;
	}

	closeDatabase();
}

long long int DatabaseHelper::getTotalBytesSent()
{
	openDatabase();

	long long int totalBytesSent = 0;
	std::string query = "SELECT total_bytes_sent FROM stats;";
	sqlite3_stmt* stmt;

	int result = sqlite3_prepare_v2(m_database, query.c_str(), -1, &stmt, nullptr);
	if (result != SQLITE_OK)
	{
		throw std::exception("Failed to execute query.");
	}

	if (sqlite3_step(stmt) == SQLITE_ROW)
	{
		totalBytesSent = sqlite3_column_int64(stmt, 0);
	}

	sqlite3_finalize(stmt);
	closeDatabase();

	return totalBytesSent;
}

long long int DatabaseHelper::getTotalBytesReceived()
{
	openDatabase();

	long long int totalBytesReceived = 0;
	std::string query = "SELECT total_bytes_received FROM stats;";
	sqlite3_stmt* stmt;

	int result = sqlite3_prepare_v2(m_database, query.c_str(), -1, &stmt, nullptr);
	if (result != SQLITE_OK)
	{
		throw std::exception("Failed to execute query.");
	}

	if (sqlite3_step(stmt) == SQLITE_ROW)
	{
		totalBytesReceived = sqlite3_column_int64(stmt, 0);
	}

	sqlite3_finalize(stmt);
	closeDatabase();

	return totalBytesReceived;
}

int DatabaseHelper::getTotalConnections()
{
	openDatabase();

	int totalConnections = 0;
	std::string query = "SELECT total_connections FROM stats;";
	sqlite3_stmt* stmt;

	int result = sqlite3_prepare_v2(m_database, query.c_str(), -1, &stmt, nullptr);
	if (result != SQLITE_OK)
	{
		throw std::exception("Failed to execute query.");
	}

	if (sqlite3_step(stmt) == SQLITE_ROW)
	{
		totalConnections = sqlite3_column_int(stmt, 0);
	}

	sqlite3_finalize(stmt);
	closeDatabase();

	return totalConnections;
}

int DatabaseHelper::getTotalNodes()
{
	openDatabase();

	int totalNodes = 0;
	std::string query = "SELECT total_nodes FROM stats;";
	sqlite3_stmt* stmt;

	int result = sqlite3_prepare_v2(m_database, query.c_str(), -1, &stmt, nullptr);
	if (result != SQLITE_OK)
	{
		throw std::exception("Failed to execute query.");
	}

	if (sqlite3_step(stmt) == SQLITE_ROW)
	{
		totalNodes = sqlite3_column_int(stmt, 0);
	}

	sqlite3_finalize(stmt);
	closeDatabase();

	return totalNodes;
}

int DatabaseHelper::getTotalClients()
{
	openDatabase();

	int totalClients = 0;
	std::string query = "SELECT total_clients FROM stats;";
	sqlite3_stmt* stmt;

	int result = sqlite3_prepare_v2(m_database, query.c_str(), -1, &stmt, nullptr);
	if (result != SQLITE_OK)
	{
		throw std::exception("Failed to execute query.");
	}

	if (sqlite3_step(stmt) == SQLITE_ROW)
	{
		totalClients = sqlite3_column_int(stmt, 0);
	}

	sqlite3_finalize(stmt);
	closeDatabase();

	return totalClients;
}

void DatabaseHelper::resetStats()
{
	openDatabase();

	char* error_message = nullptr;

	string query = "UPDATE stats SET total_bytes_sent = 0, total_bytes_received = 0, total_connections = 0, total_nodes = 0, total_clients = 0;";
	int rc = sqlite3_exec(m_database, query.c_str(), NULL, 0, &error_message);

	if (rc != SQLITE_OK)
	{
		cout << error_message << endl;
		sqlite3_free(error_message);
	}
	else
	{
		cout << "Stats reset successfully\n" << endl;
	}

	closeDatabase();
}

void DatabaseHelper::printStats()
{
	cout << "Total bytes sent: " << getTotalBytesSent() << endl;
	cout << "Total bytes received: " << getTotalBytesReceived() << endl;
	cout << "Total connections: " << getTotalConnections() << endl;
	cout << "Total nodes: " << getTotalNodes() << endl;
	cout << "Total clients: " << getTotalClients() << endl;
}