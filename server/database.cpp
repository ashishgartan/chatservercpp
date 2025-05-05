// 📁 database.cpp
#include "database.h"
#include <iostream>
#include <sqlite3.h>
#include <mutex>
#include <unordered_set>
#include <arpa/inet.h>
#include <../packets/Packet.h>
#include <map>
#include <string>

// ANSI color codes
#define GREEN "\033[32m"
#define CYAN "\033[36m"
#define RED "\033[31m"
#define YELLOW "\033[33m"
#define RESET "\033[0m"

sqlite3 *db;                                 // ✅ Real variable lives here.
std::mutex valid_users_mutex;                // ✅ Real variable lives here.
std::unordered_set<std::string> valid_users; // ✅ Real variable lives here.

void init_database()
{
    std::cout << CYAN << "📡 Connecting to SQLite database..." << RESET << std::endl;
    if (sqlite3_open("chat_server.db", &db) != SQLITE_OK)
    {
        std::cerr << RED << "[ERROR] ❌ Database connection failed\n"
                  << RESET;
        exit(1);
    }
    std::cout << GREEN << "✅ Connected to SQLite database successfully!" << RESET << std::endl;

    sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS users (user_id TEXT PRIMARY KEY, password TEXT);", nullptr, nullptr, nullptr);
    sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS messages (sender TEXT, receiver TEXT, message TEXT, timestamp DATETIME DEFAULT CURRENT_TIMESTAMP, delivered INTEGER DEFAULT 0);", nullptr, nullptr, nullptr);
    std::cout << GREEN << "✅ Tables ensured (users, messages)!" << RESET << std::endl;
}

void load_all_users()
{
    std::cout << CYAN << "🔄 Loading users from DB into memory..." << RESET << std::endl;
    std::lock_guard<std::mutex> lock(valid_users_mutex);
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, "SELECT user_id FROM users;", -1, &stmt, nullptr) == SQLITE_OK)
    {
        int count = 0;
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            valid_users.insert(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0)));
            count++;
        }
        sqlite3_finalize(stmt);
        std::cout << GREEN << "✅ " << count << " users loaded into memory." << RESET << std::endl;
    }
    else
    {
        std::cerr << RED << "⚠️ Failed to load users from database!" << RESET << std::endl;
    }
}

void save_message(const std::string &sender, const std::string &receiver, const std::string &message)
{
    std::string query = "INSERT INTO messages (sender, receiver, message) VALUES ('" + sender + "', '" + receiver + "', '" + message + "');";
    sqlite3_exec(db, query.c_str(), nullptr, nullptr, nullptr);
    std::cout << YELLOW << "💬 Message saved from " << sender << " to " << receiver << RESET << std::endl;
}

std::string get_chat_history(const std::string &user_id_1, const std::string &user_id_2)
{
    std::string query =
        "SELECT sender, receiver, message, timestamp FROM messages "
        "WHERE (sender = '" +
        user_id_1 + "' AND receiver = '" + user_id_2 + "') "
                                                       "OR (sender = '" +
        user_id_2 + "' AND receiver = '" + user_id_1 + "') "
                                                       "ORDER BY timestamp ASC;";

    sqlite3_stmt *stmt;
    std::string history;

    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
    {
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            std::string timestamp = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3));
            std::string sender = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
            std::string message = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));

            if (sender == user_id_1)
            {
                // Left aligned (you)
                history += user_id_1 + ": " + message + "↩️";
            }
            else
            {
                // Right aligned (other user)
                // std::string padded = std::string(60, ' ') + user_id_2 + ": " + message;
                history += user_id_2 + ": " + message + "↩️";
            }
        }
        sqlite3_finalize(stmt);
        std::cout << CYAN << "📜 Chat history (formatted) fetched between " << user_id_1 << " and " << user_id_2 << RESET << std::endl;
    }
    else
    {
        std::cerr << RED << "❌ Failed to retrieve formatted chat history between " << user_id_1 << " and " << user_id_2 << RESET << std::endl;
    }
    return history;
}
void send_chat_history_in_chunks(const std::string &user_id_1, const std::string &user_id_2, int client_socket)
{
    std::string history = get_chat_history(user_id_1, user_id_2); // Get full chat history
    std::cout << "📜 Total history size: " << history.size() << " bytes\n";
    size_t chunk_size = 2048; // Max packet size (2KB including header)

    // The header (packetType|queryType|isLastChunk) can take up to 100 bytes (adjust as needed)
    size_t header_size = 4;

    // Calculate the available space for the data (taking into account the header size)
    size_t available_data_size = chunk_size - header_size;
    size_t history_size = history.size();
    size_t chunks_count = (history_size / available_data_size) + (history_size % available_data_size == 0 ? 0 : 1);
    std::cout << "📦 Number of chunks to send: " << chunks_count << "\n";
    for (size_t i = 0; i < chunks_count; ++i)
    {
        AeroProtocolPacket queryPacket;
        queryPacket.header.packetType = 2; // Assuming 2 is for "chat history"

        // Calculate start and end positions for this chunk's data
        size_t start = i * available_data_size;
        size_t end = std::min((i + 1) * available_data_size, history_size);
        std::string subhistory = history.substr(start, end - start);
        std::vector<char> data(subhistory.begin(),subhistory.end());
        queryPacket.data = data;

        // Serialize and send the packet
        auto serialized_chunk = queryPacket.serialize();
        // std::cout << serialized_chunk;
        if (send(client_socket, serialized_chunk.data(), serialized_chunk.size(), 0) == -1)
        {
            perror("❌ Send failed");
            break;
        }
        std::cout << "📦 Sent chunk " << i + 1 << "/" << chunks_count << "\n";
    }
    std::cout << "Whole history of " << user_id_1 << " with " << user_id_2 << " sent..." << std::endl;
    ;
}

std::string fetch_undelivered_messages(const std::string &receiver)
{
    std::string query = "SELECT sender, message, timestamp FROM messages WHERE receiver = '" + receiver + "' AND delivered = 0 ORDER BY timestamp ASC;";
    sqlite3_stmt *stmt;
    std::string result;
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
    {
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            result += reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
            result += " | ";
            result += reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
            result += ": ";
            result += reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
            result += "\n";
        }
        sqlite3_finalize(stmt);
        std::cout << YELLOW << "📩 Undelivered messages fetched for " << receiver << RESET << std::endl;
    }
    else
    {
        std::cerr << RED << "❌ Could not fetch undelivered messages for " << receiver << RESET << std::endl;
    }
    return result;
}

void mark_messages_delivered(const std::string &receiver)
{
    std::string query = "UPDATE messages SET delivered = 1 WHERE receiver = '" + receiver + "';";
    sqlite3_exec(db, query.c_str(), nullptr, nullptr, nullptr);
    std::cout << GREEN << "✅ Messages marked as delivered for " << receiver << RESET << std::endl;
}

bool delete_account(std::string user_id, int client_socket)
{
    std::cout << "⚙️  Received delete_account request from user ID: " << user_id << std::endl;

    std::string delete_user_sql = "DELETE FROM users WHERE user_id = '" + user_id + "';";

    char *errMsg = nullptr;
    int rc = sqlite3_exec(db, delete_user_sql.c_str(), nullptr, nullptr, &errMsg);

    if (rc != SQLITE_OK)
    {
        std::cerr << RED << "❌ Error deleting user: " << errMsg << RESET << std::endl;
        sqlite3_free(errMsg);
        return false;
    }

    std::cout << GREEN << "✅ Successfully deleted user ID: " << user_id << RESET << std::endl;
    return true;
}
