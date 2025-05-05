// 📁 auth.h
#pragma once
#include <mutex>
#include <map>
#include <string>
#include <mutex>

#include "../packets/PacketType.h"
#include "../packets/Packet.h"
extern std::mutex user_mutex;
extern std::map<std::string, std::string> users;

AeroProtocolPacket register_user(AeroProtocolPacket &authPacket);
AeroProtocolPacket login_user(AeroProtocolPacket &authPacket);
