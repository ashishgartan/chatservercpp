#include "LoginSignupPacket.h"
#include <sstream>
#include <iostream>

LoginSignupPacket::LoginSignupPacket(const std::string &id, const std::string &pass) : user_id(id), password(pass) {}

std::string LoginSignupPacket::serialize() const
{
    std::stringstream ss;
    ss << packetType << "|" << user_id << "|" << password;
    return ss.str();
}

void LoginSignupPacket::deserialize(const std::string &str)
{
    std::stringstream ss(str);
    std::string token;

    // Read packetType
    std::getline(ss, token, '|');
    packetType = std::stoi(token);

    // Read user_id
    std::getline(ss, user_id, '|');

    // Read password
    std::getline(ss, password, '|');

}
