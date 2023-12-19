#pragma once

#include<string>
#include "ClientData.hpp"
#include "nlohmann/json.hpp"

using std::string;

class DataObject {
public:
	DataObject() {
		this->data = "";
		this->sender = ClientData();
		this->time = 0;
		this->hops = 0;
		this->encryptionRounds = 0;
	}

	DataObject(string data, ClientData sender, unsigned long long time, int hops = 0, int encryptionRounds = 5) {
		this->data = data;
		this->sender = sender;
		this->time = time;
		this->hops = hops;
		this->encryptionRounds = encryptionRounds;
	}

	const string& getData() const {
		return data;
	}

	void setData(const string& newData) {
		data = newData;
	}

	const ClientData& getSender() const {
		return sender;
	}

	void setSender(const ClientData& newSender) {
		sender = newSender;
	}

	unsigned long long getTime() const {
		return time;
	}

	void setTime(unsigned long long newTime) {
		time = newTime;
	}

	int getHops() const {
		return hops;
	}

	void setHops(int newHops) {
		hops = newHops;
	}

	void increaseHops() {
		hops++;
	}

	int getEncryptionRounds() const {
		return encryptionRounds;
	}

	void setEncryptionRounds(int newEncryptionRounds) {
		encryptionRounds = newEncryptionRounds;
	}

	void increaseEncryptionRounds() {
		encryptionRounds++;
	}

	string toString() const {
		nlohmann::json jsonObj;
		jsonObj["data"] = data;
		jsonObj["sender"] = sender.toString();
		jsonObj["time"] = time;
		jsonObj["hops"] = hops;
		jsonObj["encryptionRounds"] = encryptionRounds;

		return jsonObj.dump();
	}

	static DataObject fromString(const string& jsonString) {
		nlohmann::json jsonObj = nlohmann::json::parse(jsonString);

		DataObject newDataObject;
		newDataObject.setData(jsonObj["data"].get<string>());
		newDataObject.setSender(ClientData::fromString(jsonObj["sender"]));
		newDataObject.setTime(jsonObj["time"].get<unsigned long long>());
		newDataObject.setHops(jsonObj["hops"].get<int>());
		newDataObject.setEncryptionRounds(jsonObj["encryptionRounds"].get<int>());

		return newDataObject;
	}

	// Operator == overloading
	bool operator==(const DataObject& other) const {
		return (this->data == other.data && this->sender == other.sender && this->time == other.time && this->hops == other.hops && this->encryptionRounds == other.encryptionRounds);
	}

private:
	string data;
	ClientData sender;
	unsigned long long time;
	int hops = 0;
	int encryptionRounds;
};

