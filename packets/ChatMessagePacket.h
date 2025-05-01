#pragma once
#include <string>
#include "PacketType.h"
class ChatMessagePacket
{
public:
    int packetType = 3;
    std::string sender;
    std::string receiver;
    std::string message;

    ChatMessagePacket(const std::string &sender = "", const std::string &receiver = "", const std::string &message = "");
    
    std::string serialize() const;
    void deserialize(const std::string &str);
};
