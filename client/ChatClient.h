#pragma once
#include <thread>
#include <string>
#include <arpa/inet.h>
#include "../packets/FilePacket.h"

class ChatClient
{
public:
    ChatClient();
    void start();
    ~ChatClient();

private:
    void receiveMessages();
    void sendMessages();

    // Function to send a file by chunks using FilePackets
    void send_file_by_chunks(const std::string &receiver, const std::string &full_path, int client_socket);
    void receive_data(FilePacket filePacket);

    std::string user_id;
    int client_socket;
    struct sockaddr_in server_addr;
    std::thread send_thread, receive_thread;
    bool is_running;
};
