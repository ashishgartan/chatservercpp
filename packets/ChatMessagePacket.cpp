#include "ChatMessagePacket.h"
#include <sstream>
#include <iostream>

ChatMessagePacket::ChatMessagePacket(const std::string &sender, const std::string &receiver, const std::string &message)
    : sender(sender), receiver(receiver), message(message) {}

std::string ChatMessagePacket::serialize() const
{
    std::stringstream ss;
    ss << packetType << "|" << sender << "|" << receiver << "|" << message;
    return ss.str();
}

void ChatMessagePacket::deserialize(const std::string &str)
{
    std::stringstream ss(str);
    std::string token;

    // Read packetType
    std::getline(ss, token, '|');
    packetType = std::stoi(token);

    // Read sender
    std::getline(ss, sender, '|');

    // Read receiver
    std::getline(ss, receiver, '|');

    // Read message
    std::getline(ss, message);

   }
