// 📁 handlers.cpp
#include "handlers.h"
#include "auth.h"
#include "database.h"
#include "utils.h"
#include <iostream>
#include <sstream>
#include <string>
#include <cstring>
#include <thread>
#include <mutex>
#include <unistd.h>
#include <arpa/inet.h>
#include <map>
#include <unordered_set>
#include <fstream>
#include "../packets/QueryPacket.h"
#include "../packets/LoginSignupPacket.h"
#include "../packets/ChatMessagePacket.h"
#include "../packets/FilePacket.h"
#include "../packets/ACKPacket.h"
#include "../packets/PacketType.h"
/**
 *  LOGIN_PACKET = 1,
    REGISTER_PACKET = 2,
    CHAT_MESSAGE_PACKET = 3,
    FILE_PACKET = 4,
    ACK_PACKET = 5,
    QUERY_PACKET = 6
 */
/*
Command	        Query Code	    Example Usage
/logout	        1	        	/logout
/history	    2	        	/history
/delete_account	3	            /delete_account
/online_users	4	            /online_users
/clear_chat	    5	            /clear_chat
/help       	6	            /help
/whoami	        7	        	/whoami
/sendfile	    8	            /sendfile
/broadcast	    9	            /broadcast Hello everyone!
/block [user]	10	            /block user123
/unblock [user]	11	            /unblock user123
/profile	    12	            /profile
/changepass	    13	            /changepass newpassword123

*/
#define GREEN "\033[32m"
#define CYAN "\033[36m"
#define RED "\033[31m"
#define YELLOW "\033[33m"
#define RESET "\033[0m"
#define PORT 8080     // 🌐 Server will listen on port 8080
#define MSG_SIZE 1024 // 💬 Max message size for normal text messages

// 👥 In-memory storage for currently connected clients: lsp.user_id -> socket
std::map<std::string, int> clients;
// 🔐 Mutex for synchronizing access to clients map
std::mutex client_mutex;

// 🚀 Handles a single client after connection is accepted
void handle_client(int client_socket)
{
    char buffer[MSG_SIZE];
    memset(buffer, 0, sizeof(buffer));

    // 🔐 Step 1: Receive login or registration request
    int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
    if (bytes_received <= 0)
    {
        std::cout << "❌ Failed to receive data from client. Closing connection.\n";
        close(client_socket);
        return;
    }

    LoginSignupPacket lsp;
    lsp.deserialize(buffer);
    ACKPacket ackPacket;

    // 🧾 Step 2: Authenticate or Register user
    if (lsp.packetType == 2)
    {
        std::cout << "📜 Registering user: " << lsp.user_id << "\n";
        ackPacket = register_user(lsp);
    }
    else if (lsp.packetType == 1)
    {
        std::cout << "🔐 Login attempt for user: " << lsp.user_id << "\n";
        // here also implement logic of whether we have logged in any other session or not?
        // if yes then use any logic to handle it .
        ackPacket = login_user(lsp);
        // ✅ Step 3: Save client as online
        clients[lsp.user_id] = client_socket;
        std::lock_guard<std::mutex> lock(client_mutex);
        std::cout << " ✅ User " << lsp.user_id << " logged in successfully. Current online users: \n";
        for (const auto &[user, _] : clients)
        {
            std::cout << user << " ";
        }
        std::cout << "\n";
    }

    std::string serialized_response = ackPacket.serialize();
    send(client_socket, serialized_response.c_str(), serialized_response.size(), 0);
    if (ackPacket.status == 0)
    {
        std::cout << "❌ Login/Registration failed for user: " << lsp.user_id << "\n";
        close(client_socket);
        return;
    }

    // // Send undelivered messages if any
    // std::string offline_msgs = fetch_undelivered_messages(lsp.user_id);
    // if (!offline_msgs.empty())
    // {
    //     offline_msgs = "@@NOTIFY " + offline_msgs;
    //     send(client_socket, offline_msgs.c_str(), offline_msgs.length(), 0);
    //     mark_messages_delivered(lsp.user_id);
    //     std::cout << "📩 Sent undelivered messages to user: " << lsp.user_id << "\n";
    // }

    // 📡 Step 5: Start listening for client messages
    while (true)
    {
        char received_serialized_data[2048] = {0};
        int bytes_read = recv(client_socket, received_serialized_data, sizeof(received_serialized_data), 0);
        if (bytes_read <= 0)
        {
            std::cout << "❌ Client " << lsp.user_id << " disconnected.\n";
            break;
        }

        int type = std::stoi(std::string(1, received_serialized_data[0]));

        // 📦 Handle commands like /history, /logout, etc.
        if (type == 6)
        {
            QueryPacket queryPacket;
            queryPacket.deserialize(received_serialized_data);
            int queryType = queryPacket.queryType;
            if (queryType == 0)
            {
                std::string user_id = queryPacket.data;

                // 🔍 Check if the user is in the clients map
                queryPacket.data = (clients.find(user_id) != clients.end()) ? "Online" : "Offline";
                serialized_response = queryPacket.serialize();
                send(client_socket, serialized_response.c_str(), serialized_response.size(), 0);
            }
            else if (queryType == 1)
            {
                std::lock_guard<std::mutex> lock(client_mutex);
                clients.erase(lsp.user_id);
                queryPacket.data = "Logged out successfully";
                serialized_response = queryPacket.serialize();
                send(client_socket, serialized_response.c_str(), serialized_response.size(), 0);
                std::cout << "✅ User " << lsp.user_id << " logged out successfully.\n";
                break;
            }
            else if (queryType == 2)
            {
                std::string receiver_id = queryPacket.data;
                std::string sender_id = lsp.user_id;
                send_chat_history_in_chunks(sender_id, receiver_id, clients[sender_id]);
            }
            else if (queryType == 3)
            {
                std::cout << YELLOW << "⚙️  Received delete_account request from user ID: "
                          << lsp.user_id << RESET << std::endl;

                bool deleteSuccess = delete_account(lsp.user_id, clients[lsp.user_id]);

                if (deleteSuccess)
                {
                    queryPacket.data = "1";
                    std::cout << GREEN << "✔️  Account deleted for user ID: "
                              << lsp.user_id << RESET << std::endl;
                }
                else
                {
                    queryPacket.data = "0";
                    std::cout << RED << "❌ Error deleting account for user ID: "
                              << lsp.user_id << RESET << std::endl;
                }

                serialized_response = queryPacket.serialize();

                send(client_socket, serialized_response.c_str(), serialized_response.size(), 0);

                std::cout << CYAN << "📤 Response sent to client: " << queryPacket.data
                          << RESET << std::endl;

                if (deleteSuccess)
                {
                    // ✅ Close client connection
                    std::cout << YELLOW << "🔌 Disconnecting user ID: " << lsp.user_id << RESET << std::endl;

                    // ✅ Remove from map safely
                    {
                        std::lock_guard<std::mutex> lock(client_mutex);
                        clients.erase(lsp.user_id);
                    }

                    close(client_socket); // Close the socket
                    return;               // Exit the thread
                }
            }
            else if (queryType == 4)
            {
                std::string onlineUsers = get_online_users();
                queryPacket.data = onlineUsers;
                serialized_response = queryPacket.serialize();
                std::cout << "📡 Sending online users list to user: " << lsp.user_id << "\n";
                send(client_socket, serialized_response.c_str(), serialized_response.length(), 0);
            }
            else if (queryType == 5)
            {
            }
            else if (queryType == 6)
            {
            }
            else if (queryType == 6)
            {
            }
            else if (queryType == 6)
            {
            }
            else if (queryType == 7)
            {
            }
            else if (queryType == 8)
            {
            }
            else if (queryType == 9)
            {
                ChatMessagePacket chatPacket;
                for (auto &[user_id, socket] : clients)
                {
                    if (client_socket != socket)
                    {
                        chatPacket.sender = user_id;
                    }
                }
                for (auto &[user_id, socket] : clients)
                {
                    if (client_socket != socket)
                    {
                        chatPacket.message = queryPacket.data;
                        serialized_response = chatPacket.serialize();
                        send(socket, serialized_response.c_str(), serialized_response.size(), 0);
                    }
                }
            }
            else if (queryType == 10)
            {
            }
            else if (queryType == 11)
            {
            }
            else if (queryType == 12)
            {
            }
            else if (queryType == 13)
            {
            }
            continue;
        }

        // 🖼️ Handle File transfer
        if (type == 4)
        {
            FilePacket filePacket;
            filePacket.deserialize(received_serialized_data);
            std::cout << "📁 File transfer from " << lsp.user_id << " to " << filePacket.receiver << "\n";

            // 1️⃣ Notify receiver if online
            {
                std::lock_guard<std::mutex> lock(client_mutex);
                if (clients.count(filePacket.receiver))
                {
                    int receiver_socket = clients[filePacket.receiver];
                    send(receiver_socket, received_serialized_data, sizeof(received_serialized_data), 0);
                    std::cout << "📤 File sent to online receiver: " << filePacket.receiver << "\n";
                }
                else
                {
                    std::cout << "❌ Receiver " << filePacket.receiver << " is offline. File not sent.\n";
                }
            }
            continue;
        }

        // 💬 Standard message (user-to-user)
        if (type == 3)
        {
            ChatMessagePacket chatPacket;
            chatPacket.deserialize(received_serialized_data);
            save_message(lsp.user_id, chatPacket.receiver, chatPacket.message);
            std::cout << "💬 Message from " << lsp.user_id << " to " << chatPacket.receiver << ": " << chatPacket.message << "\n";

            {
                std::lock_guard<std::mutex> lock(client_mutex);
                if (clients.count(chatPacket.receiver))
                {
                    send(clients[chatPacket.receiver], received_serialized_data, sizeof(received_serialized_data), 0);
                    std::cout << "📤 Message sent to online receiver: " << chatPacket.receiver << "\n";
                }
                else
                {
                    ACKPacket ackPacket;
                    ackPacket.message = "User offline. Message stored.";
                    std::string serialized_response = ackPacket.serialize();
                    send(client_socket, serialized_response.c_str(), serialized_response.length(), 0);
                    std::cout << "❌ User " << chatPacket.receiver << " is offline. Message stored for later delivery.\n";
                }
            }
            continue;
        }
    }

    // ❌ Client disconnected
    {
        std::lock_guard<std::mutex> lock(client_mutex);
        clients.erase(lsp.user_id);
    }
    close(client_socket);
    std::cout << "❌ Client " << lsp.user_id << " connection closed.\n";
}

// 🌐 Start the TCP server and accept incoming connections
void start_server()
{
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(server_socket, 1);

    std::cout << "🚀 Server started on port " << PORT << "\n";

    // 🔄 Accept clients continuously
    while (true)
    {
        struct sockaddr_in client_addr;
        socklen_t client_addr_size = sizeof(client_addr);
        int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_size);
        std::thread client_thread(handle_client, client_socket);
        client_thread.detach(); // 🧵 Run each client in its own thread
    }
}

// 👥 Get list of currently online users
std::string get_online_users()
{
    std::lock_guard<std::mutex> lock(client_mutex);
    std::string result = "👥 Online users:";
    for (const auto &[user, _] : clients)
    {
        result += " | " + user;
    }
    std::cout << result;
    return result;
}
