#pragma once

#include "../ServerProject/Constants.h"
#include "../ServerProject/nlohmann/json.hpp"
#include <string>


using std::string;
using json = nlohmann::json;

class JsonResponse
{
public:
	JsonResponse(string nodeIp, unsigned short nodePort, string nodeConversationId, int messageCode, string senderIp, unsigned short senderPort, string message);
	JsonResponse(string formattedJsonString);
	~JsonResponse();

	bool isErrorResponse() const;
	static bool isErrorCode(int messageCode);

	string toString() const;
	static JsonResponse fromString(string jsonString);

	friend std::ostream& operator<<(std::ostream& os, const JsonResponse& response);

private:
	string nodeIp;
	unsigned short nodePort;
	string nodeConversationId;
	int messageCode;
	string senderIp;
	unsigned short senderPort;
	string message;
	string response;
};
