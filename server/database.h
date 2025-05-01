// 📁 database.h
#pragma once
#include <string>
#include <mutex>
#include <unordered_set>
#include <sqlite3.h>

extern sqlite3 *db; // ✅ Declaration shared across project
extern std::mutex valid_users_mutex;// ✅ Declaration shared across project
extern std::unordered_set<std::string> valid_users;// ✅ Declaration shared across project

void init_database();
void load_all_users();
void save_message(const std::string &sender, const std::string &receiver, const std::string &message);
std::string get_chat_history(const std::string &user_id_1,const std::string &user_id_2);
void send_chat_history_in_chunks(const std::string &user_id_1, const std::string &user_id_2, int client_socket);
std::string fetch_undelivered_messages(const std::string &receiver);
void mark_messages_delivered(const std::string &receiver);
bool delete_account(std::string user_id,int client_socket);
    