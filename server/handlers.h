// 📁 handlers.h
#pragma once
#include <map>
#include <mutex>
#include <string>
extern std::map<std::string, int> clients;
extern std::mutex client_mutex;

void start_server();
void handle_client(int client_socket);
std::string get_online_users();
