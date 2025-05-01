#include "ACKPacket.h"
#include <sstream>
#include <iostream>

ACKPacket::ACKPacket(int s, const std::string &msg) : status(s), message(msg) {}

std::string ACKPacket::serialize() const
{
    std::stringstream ss;
    ss << packetType << "|" << status << "|" << message;
    return ss.str();
}

void ACKPacket::deserialize(const std::string &str)
{
    std::stringstream ss(str);
    std::string token;

    // Read packetType
    std::getline(ss, token, '|');
    packetType = std::stoi(token);

    // Read status
    std::getline(ss, token, '|');
    status = std::stoi(token);

    // Read message
    std::getline(ss, message);
}
