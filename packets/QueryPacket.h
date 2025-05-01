#pragma once
#include <string>
#include "PacketType.h"
class QueryPacket
{
public:
    int packetType = 6;
    int queryType;
    std::string data;

    QueryPacket(const std::string &d = "");
    
    std::string serialize() const;
    void deserialize(const std::string &str);
};
