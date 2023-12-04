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

bool DatabaseHelper::openDatabase()
{
	int rc = sqlite3_open(filename.c_str(), &m_database);
	if (rc)
	{
		Log::error("Can't open database: ");
		Log::error(sqlite3_errmsg(m_database));
		return false;
	}
	else
	{
		Log::action("Opened database successfully\n");
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

	// Create tables
	string create_table_query = "CREATE TABLE IF NOT EXISTS clients (username TEXT PRIMARY KEY, password TEXT, email TEXT, bytes_sent INTEGER, bytes_received INTEGER, aes_key TEXT, aes_iv TEXT);";
	int rc = sqlite3_exec(m_database, create_table_query.c_str(), NULL, 0, &error_message);
	if (rc != SQLITE_OK)
	{
		Log::error(error_message);
		sqlite3_free(error_message);
	}
	else
	{
		Log::action("Table created successfully\n");
	}

	create_table_query = "CREATE TABLE IF NOT EXISTS nodes (server_port TEXT PRIMARY KEY, time TEXT, connection TEXT, aes_key TEXT, aes_iv TEXT);";
	rc = sqlite3_exec(m_database, create_table_query.c_str(), NULL, 0, &error_message);
	if (rc != SQLITE_OK)
	{
		Log::error(error_message);
		sqlite3_free(error_message);
	}
	else
	{
		Log::action("Table created successfully\n");
	}

	create_table_query = "CREATE TABLE IF NOT EXISTS stats (total_bytes_sent INTEGER, total_bytes_received INTEGER, total_connections INTEGER, total_nodes INTEGER, total_clients INTEGER, total_stats INTEGER);";
	rc = sqlite3_exec(m_database, create_table_query.c_str(), NULL, 0, &error_message);
	if (rc != SQLITE_OK)
	{
		Log::error(error_message);
		sqlite3_free(error_message);
	}
	else
	{
		Log::action("Table created successfully\n");
	}

	closeDatabase();
}