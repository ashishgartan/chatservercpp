#include "QueryPacket.h"
#include <sstream>
#include <iostream>

QueryPacket::QueryPacket(const std::string &d) : data(d) {}

std::string QueryPacket::serialize() const
{
    std::stringstream ss;
    ss << packetType << "|" << queryType << "|" << data;  // Serialize packetType, queryType, and data
    return ss.str();
}

void QueryPacket::deserialize(const std::string &str)
{
    std::stringstream ss(str);
    std::string token;

    // Read packetType
    std::getline(ss, token, '|');
    packetType = std::stoi(token);

    // Read queryType
    std::getline(ss, token, '|');
    queryType = std::stoi(token);

    // Read data
    std::getline(ss, data);
}
