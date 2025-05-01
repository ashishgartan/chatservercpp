#pragma once
#include <string>
#include "PacketType.h"
class LoginSignupPacket
{
public:
    int packetType = 1;
    std::string user_id;
    std::string password;

    LoginSignupPacket(const std::string &id = "", const std::string &pass = "");
    
    std::string serialize() const;
    void deserialize(const std::string &str);
};
