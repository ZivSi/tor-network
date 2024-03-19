#include "Logger.h"


Logger::Logger(string identifier) {
	this->identifier = identifier;

	timeFormat = "[%H:%M:%S::%3f] ";
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

string getCurrentTime() {
	// Get current time point
	auto now = std::chrono::system_clock::now();

	// Convert to time_t (seconds since epoch)
	std::time_t now_time = std::chrono::system_clock::to_time_t(now);


	std::tm time_info;
	localtime_s(&time_info, &now_time);

	// Get milliseconds
	auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() % 1000;

	// Format the time
		// Format the time
	std::ostringstream oss;
	oss << std::put_time(&time_info, "%H:%M:%S") << ":" << std::setw(3) << std::setfill('0') << milliseconds;

	return oss.str();
}

void Logger::printLastLogMessage()
{
	try {
		if (!logMessages.empty()) {
			cout << getCurrentTime() << " | " << logMessages.back() << endl;
		}
	}
	catch (const std::exception& e) {
		std::cout << "Error in Logger::printLastLogMessage: " << e.what() << std::endl;
	}
	catch (...) {
		std::cout << "Error in Logger::printLastLogMessage: Unknown error" << std::endl;
	}
}
vector<string>* Logger::getLogMessages()
{
	return &logMessages;
}

