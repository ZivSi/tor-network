#pragma once

#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using std::string;
using std::cout;
using std::endl;
using std::vector;

class Logger
{
private:
	vector<string> logMessages;
	string identifier;
	string timeFormat;

public:
	Logger(string identifier);
	~Logger();

	void log(string message);
	void keysInfo(string message);
	void success(string message);
	void clientEvent(string message);
	void error(string message);
	void formatError(string message);
	void warning(string message);
	void clearLog();

	void printLog();
	void printLastLogMessage();

	vector<string>* getLogMessages();
};

