// 📁 auth.h
#pragma once
#include <mutex>
#include <map>
#include <string>
#include <mutex>
#include "../packets/QueryPacket.h"
#include "../packets/LoginSignupPacket.h"
#include "../packets/ChatMessagePacket.h"
#include "../packets/FilePacket.h"
#include "../packets/ACKPacket.h"
#include "../packets/PacketType.h"
extern std::mutex user_mutex;
extern std::map<std::string, std::string> users;

ACKPacket register_user(LoginSignupPacket lsp);
ACKPacket login_user(LoginSignupPacket lsp);
