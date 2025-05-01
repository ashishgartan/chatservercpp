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

#include "../packets/LoginSignupPacket.h"
#include "../packets/ACKPacket.h"

ACKPacket register_user(LoginSignupPacket lsp)
{
    std::lock_guard<std::mutex> lock(user_mutex);
    std::cout << "[🔐] Register request received for user: " << lsp.user_id << std::endl;

    std::string hashed_password = hash_password(lsp.password);
    std::string query = "INSERT INTO users (user_id, password) VALUES ('" + lsp.user_id + "', '" + hashed_password + "');";

    ACKPacket ackPacket;
    int rc = sqlite3_exec(db, query.c_str(), nullptr, nullptr, nullptr);

    if (rc == SQLITE_OK)
    {
        std::cout << "[✅] Registration successful for user: " << lsp.user_id << std::endl;
        ackPacket.status = 1;
        ackPacket.error_code = 0;
        ackPacket.message = "Registration successful";
    }
    else
    {
        std::cout << "[❌] Registration failed for user: " << lsp.user_id << ". Possible reason: User already exists or database error." << std::endl;
        ackPacket.status = 0;
        ackPacket.error_code = 3;
        ackPacket.message = "Registration failed: User already exists";
    }

    return ackPacket;
}

ACKPacket login_user(LoginSignupPacket lsp)
{
    std::lock_guard<std::mutex> lock(user_mutex);
    std::cout << "[🔐] Login request received for user: " << lsp.user_id << std::endl;
    
    std::string hashed_password = hash_password(lsp.password);
    std::string query = "SELECT user_id FROM users WHERE user_id = '" + lsp.user_id + "' AND password = '" + hashed_password + "';";
    sqlite3_stmt *stmt;
    ACKPacket ackPacket;

    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
    {
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            std::cout << "[✅] Login successful for user: " << lsp.user_id << std::endl;
            ackPacket.status = 1;
            ackPacket.error_code = 0;
            ackPacket.message = "Login successful";
        }
        else
        {
            std::cout << "[❌] Login failed for user: " << lsp.user_id << ". Reason: Invalid credentials." << std::endl;
            ackPacket.status = 0;
            ackPacket.error_code = 2;
            ackPacket.message = "Login failed: Invalid credentials";
        }
        sqlite3_finalize(stmt);
    }
    else
    {
        std::cout << "[🔥] Login server/database error for user: " << lsp.user_id << std::endl;
        ackPacket.status = 0;
        ackPacket.error_code = 4;
        ackPacket.message = "Login failed: Server/database error";
    }

    return ackPacket;
}
