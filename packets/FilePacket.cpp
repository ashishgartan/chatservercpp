#include "FilePacket.h"
#include <sstream>
#include <iostream>

FilePacket::FilePacket(const std::string &sender, const std::string &receiver, int packet_number, size_t total_size)
    : packet_number(packet_number), sender(sender), receiver(receiver), filename(""), data(""), total_size(total_size) {}

std::string FilePacket::serialize() const
{
    std::stringstream ss;
    ss << packetType << "|" << sender << "|" << receiver << "|" << filename << "|" << packet_number << "|" << total_size << "|" << data;
    return ss.str();
}

void FilePacket::deserialize(const std::string &str)
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

    // Read filename
    std::getline(ss, filename, '|');

    // Read packet_number
    ss >> packet_number;
    char delim;
    ss >> delim;  // consume '|'

    // Read total_size
    ss >> total_size;
    ss >> delim;  // consume '|'

    // Read data
    std::getline(ss, data);
}
