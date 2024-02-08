#include "Logger.h"


Logger::Logger(string identifier) {
	this->identifier = identifier;
}

Logger::~Logger() {}

void Logger::log(string message)
{
	logMessages.push_back("\033[94m[" + this->identifier + " LOG]\033[0m " + message); // Blue color

	printLastLogMessage();
}

void Logger::keysInfo(string message)
{
	logMessages.push_back("\033[95m[" + this->identifier + " KEY]\033[0m " + message); // Purple color

	printLastLogMessage();
}

void Logger::success(string message)
{
	logMessages.push_back("\033[92m[" + this->identifier + " SUCCESS]\033[0m " + message); // Green color

	printLastLogMessage();
}

void Logger::clientEvent(std::string message) {
	logMessages.push_back("\033[94m[" + this->identifier + " CLIENT EVENT]\033[0m " + message); // Sky blue color

	printLastLogMessage();
}

void Logger::error(string message)
{
	logMessages.push_back("\033[91m[" + this->identifier + " ERROR]\033[0m " + message); // Red color

	printLastLogMessage();
}

void Logger::formatError(string message)
{
	logMessages.push_back("\033[91m[" + this->identifier + " FORMAT ERROR]\033[0m " + message); // Red color

	printLastLogMessage();
}

void Logger::warning(string message)
{
	logMessages.push_back("\033[93m[" + this->identifier + " WARNING]\033[0m " + message); // Yellow color

	printLastLogMessage();
}

void Logger::clearLog()
{
	Logger::logMessages.clear();
}

void Logger::printLog()
{
	for (string message : Logger::logMessages)
	{
		cout << message << endl;
	}
}

void Logger::printLastLogMessage()
{
	if (!logMessages.empty())
	{
		cout << logMessages.back() << endl;
	}
}
vector<string>* Logger::getLogMessages()
{
	return &logMessages;
}

