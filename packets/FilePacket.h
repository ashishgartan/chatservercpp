#pragma once
#include <string>
#include "PacketType.h"
class FilePacket
{
public:
    int packetType = 4;
    int packet_number;
    std::string sender;
    std::string receiver;
    std::string filename;
    std::string data;
    size_t total_size;

    FilePacket(const std::string &sender = "", const std::string &receiver = "", int packet_number = 0, size_t total_size = 0);
    
    std::string serialize() const;
    void deserialize(const std::string &str);
};
