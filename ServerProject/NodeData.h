#pragma once

#include "Constants.h"
#include "ECCHandler.h"
#include "Utility.h"
#include <cryptlib.h>
#include <eccrypto.h>
#include <hex.h>
#include <integer.h>
#include <string>
#include <vector>

using std::string;
using std::to_string;
using std::vector;
using CryptoPP::ECDSA;
using CryptoPP::ECP;
using CryptoPP::SHA256;
using CryptoPP::ECIES;

/*
Class that will be stored on the server.
*/

class NodeData {
private:
    // ----------------- Member Variables -----------------
    string ip;
    unsigned short port;
    ECCHandler ecchandler;
    unsigned long long lastAliveMessageTime;
    unsigned long aliveMessagesCount;
    unsigned long averageResponseTime;

public:
    // ----------------- Constructors & Destructor -----------------
    NodeData(string ip, unsigned short port);
    NodeData(string ip, unsigned short port, string serializedPublicKey);
    NodeData(string ip, unsigned short port, string serializedPublicKey, unsigned long long lastAliveMessageTime, unsigned long aliveMessagesCount, unsigned long averageResponseTime);
    NodeData();
    ~NodeData();

    // ----------------- Getters -----------------
    string getIp();
    unsigned short getPort();
    ECCHandler* getECCHandler();
    unsigned long long getLastAliveMessageTime();
    unsigned long getAliveMessagesCount();
    unsigned long getAverageResponseTime();

    // ----------------- Setters -----------------
    void setIp(string ip);
    void setPort(unsigned short port);
    void setECCKey(string serializedPublicKey);
    void setLastAliveMessageTime(unsigned long long lastAliveMessageTime);
    void updateLastAliveMessageTime();
    void setAliveMessagesCount(unsigned long aliveMessagesCount);
    void setAverageResponseTime(unsigned long averageResponseTime);

    // ----------------- Encryption -----------------
    string encrypt(string plaintext);

    // ----------------- Utility -----------------
    string toString();
    vector<unsigned char> toSend();
    bool isEmpty();

    // ----------------- Operator Overloading -----------------
    operator int() const;
    bool operator==(const NodeData& other) const;
};
