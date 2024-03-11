#include "JsonResponse.h"

JsonResponse::JsonResponse(string nodeIp, unsigned short nodePort, string nodeConversationId, int messageCode, string senderIp, unsigned short senderPort, string message) {
	this->nodeIp = nodeIp;
	this->nodePort = nodePort;
	this->nodeConversationId = nodeConversationId;
	this->messageCode = messageCode;

	if (!isErrorCode(messageCode)) {
		this->senderIp = senderIp;
		this->senderPort = senderPort;
	}
	this->message = message;
}

JsonResponse::~JsonResponse() {

}

bool JsonResponse::isErrorResponse() const
{
	return isErrorCode(messageCode);
}

bool JsonResponse::isErrorCode(int messageCode)
{
	return messageCode != 0;
}

string JsonResponse::toString() const
{
	return "{\n"
		"  \"nodeIp\": \"" + nodeIp + "\",\n"
		"  \"nodePort\": " + std::to_string(nodePort) + ",\n"
		"  \"nodeConversationId\": \"" + nodeConversationId + "\",\n"
		"  \"messageCode\": " + std::to_string(messageCode) + ",\n"
		"  \"senderIp\": \"" + senderIp + "\",\n"
		"  \"senderPort\": " + std::to_string(senderPort) + ",\n"
		"  \"message\": \"" + message + "\"\n"
		"}";
}

JsonResponse JsonResponse::fromString(string jsonString)
{
	auto json = json::parse(jsonString);

	string nodeIp = json["nodeIp"];
	unsigned short nodePort = json["nodePort"];
	string nodeConversationId = json["nodeConversationId"];
	int messageCode = json["messageCode"];
	string senderIp = json["senderIp"];
	unsigned short senderPort = json["senderPort"];
	string message = json["message"];

	return JsonResponse(nodeIp, nodePort, nodeConversationId, messageCode, senderIp, senderPort, message);
}

std::ostream& operator<<(std::ostream& os, const JsonResponse& response)
{
	os << response.toString();
	return os;
}
