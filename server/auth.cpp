// 📁 auth.cpp
#include "auth.h"
#include "utils.h"
#include "database.h"
#include <mutex>
#include <iostream>
#include <sqlite3.h>
#include <map>
#include <string>

std::mutex user_mutex;
std::map<std::string, std::string> users;

#include "../packets/PacketType.h"
#include "../packets/Packet.h"
AeroProtocolPacket register_user(AeroProtocolPacket &authPacket)
{
    std::lock_guard<std::mutex> lock(user_mutex);
    std::cout << "[🔐] Register request received for user: " << authPacket.username << std::endl;

    std::string hashed_password = hash_password(authPacket.password);
    std::string query = "INSERT INTO users (user_id, password) VALUES ('" + authPacket.username + "', '" + hashed_password + "');";

    // Prepare ACK packet
    AeroProtocolPacket ackPacket;
    ackPacket.header.packetType = ACKNOWLEDGMENT;
    ackPacket.header.timestamp = std::time(nullptr);

    // Fill ackMessage field
    if (sqlite3_exec(db, query.c_str(), nullptr, nullptr, nullptr) == SQLITE_OK)
    {
        std::cout << "[✅] Registration successful for user: " << authPacket.username << std::endl;
        ackPacket.message = "SUCCESS";
        ackPacket.statusCode = 1;

    }
    else
    {
        std::cout << "[❌] Registration failed for user: " << authPacket.username << ". User might already exist.\n";
        ackPacket.message = "FAILURE: User already exists";
        ackPacket.statusCode = 0;

    }

    return ackPacket;
}

AeroProtocolPacket login_user(AeroProtocolPacket &authPacket)
{
    std::lock_guard<std::mutex> lock(user_mutex);
    std::cout << "[🔐] Login request received for user: " << authPacket.username << std::endl;

    std::string hashed_password = hash_password(authPacket.password);
    std::string query = "SELECT user_id FROM users WHERE user_id = '" + authPacket.username + "' AND password = '" + hashed_password + "';";
    sqlite3_stmt *stmt;

    AeroProtocolPacket ackPacket;
    ackPacket.header.packetType = ACKNOWLEDGMENT;
    ackPacket.header.timestamp = std::time(nullptr);

    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
    {
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            std::cout << "[✅] Login successful for user: " << authPacket.username << std::endl;
            ackPacket.message = "SUCCESS";
            ackPacket.statusCode = 1;
        }
        else
        {
            std::cout << "[❌] Login failed for user: " << authPacket.username << ". Reason: Invalid credentials." << std::endl;
            ackPacket.message = "FAILURE: Invalid credentials";
            ackPacket.statusCode = 0;
        }
        sqlite3_finalize(stmt);
    }
    else
    {
        std::cout << "[🔥] Login server/database error for user: " << authPacket.username << std::endl;
        ackPacket.message = "FAILURE: Server/database error";
    }

    return ackPacket;
}
