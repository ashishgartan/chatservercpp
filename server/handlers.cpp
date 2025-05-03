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
#include "../packets/Packet.h"
#include "../packets/CommandType.h"


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
#define PORT 8080 // 🌐 Server will listen on port 8080
#define BUFFER_SIZE 1024

// 👥 In-memory storage for currently connected clients: lsp.user_id -> socket
std::map<std::string, int> clients;
// 🔐 Mutex for synchronizing access to clients map
std::mutex client_mutex;

// 🚀 Handles a single client after connection is accepted
void handle_client(int client_socket)
{
    // Step 1: Receive the header first
    AeroProtocolHeader header;
    int bytes_received = 0;
    while (bytes_received < sizeof(AeroProtocolHeader))
    {
        int chunk = recv(client_socket, ((char *)&header) + bytes_received, sizeof(AeroProtocolHeader) - bytes_received, 0);
        if (chunk <= 0)
        {
            std::cout << "❌ Failed to receive header data from client.\n";
            return;
        }
        bytes_received += chunk;
    }

    std::cout << "Received header: PacketType=" << header.packetType
              << ", PayloadSize=" << header.payloadSize
              << ", SequenceNumber=" << header.sequenceNumber
              << ", Timestamp=" << header.timestamp << "\n";

    // Step 2: Receive the payload based on the payload size in the header
    uint32_t payload_size = header.payloadSize;
    if (payload_size == 0)
    {
        std::cout << "No payload data to receive.\n";
        return; // No payload to process
    }

    // Receive the actual payload
    char *received_payload = new char[payload_size];
    int payload_received = 0;
    while (payload_received < payload_size)
    {
        int chunk = recv(client_socket, received_payload + payload_received, payload_size - payload_received, 0);
        if (chunk <= 0)
        {
            std::cout << "❌ Failed to receive payload data from client.\n";
            delete[] received_payload;
            return;
        }
        payload_received += chunk;
    }

    std::cout << "Received payload of size: " << payload_received << " bytes.\n";

    // Step 3: Deserialize the packet and process the data based on packet type
    AeroProtocolPacket packet;
    packet.header = header;
    packet = packet.deserialize(header, received_payload);
    delete[] received_payload; // Clean up the payload buffer after deserialization
    std::cout << "Processing packet of type: " << packet.header.packetType << std::endl;

    AeroProtocolPacket ackPacket;

    switch (packet.header.packetType)
    {
    case LOGIN_REQUEST:
    {
        std::cout << "🔐 Login attempt for user: " << packet.username << "\n";

        // Optional: logic to handle multiple logins

        ackPacket = login_user(packet); // Should return ACK packet

        // ✅ Step 3: Save client as online
        {
            std::lock_guard<std::mutex> lock(client_mutex);
            clients[packet.username] = client_socket;
            std::cout << " ✅ User " << packet.username << " logged in successfully.\nCurrent online users: \n";
            for (const auto &[user, _] : clients)
            {
                std::cout << user << " ";
            }
            std::cout << "\n";
        }
        std::vector<char> serialized_response = ackPacket.serialize();
        send(client_socket, serialized_response.data(), serialized_response.size(), 0);

        break;
    }
    case REGISTER_REQUEST:
    {
        std::cout << "📜 Registering user: " << packet.username << "\n";
        ackPacket = register_user(packet); // Should return ACK packet
        break;
    }
    case MESSAGE:
    {
        save_message(packet.senderId, packet.receiverId, packet.message);
        std::cout << "💬 Message from " << packet.senderId << " to " << packet.receiverId << ": " << packet.message << "\n";

        std::lock_guard<std::mutex> lock(client_mutex);
        if (clients.count(packet.receiverId))
        {
            ackPacket.message = "Message sent";
            std::cout << "📤 Message sent to online receiver: " << packet.receiverId << "\n";
        }
        else
        {
            ackPacket.message = "User offline. Message stored.";
            std::cout << "❌ User " << packet.receiverId << " is offline. Message stored for later delivery.\n";
        }
        std::vector<char> serialized_response = ackPacket.serialize();
        send(client_socket, serialized_response.data(), serialized_response.size(), 0);
        break;
    }
    case FILE_CHUNK:
    {
        std::cout << "📁 File packet transfer from " << packet.senderId << " to " << packet.receiverId << "\n";

        int receiver_socket = clients[packet.receiverId];
        std::vector<char> serialized_response = packet.serialize();
        send(receiver_socket, serialized_response.data(), sizeof(serialized_response.data()), 0);
        std::cout << "📤 File sent to online receiver: " << packet.receiverId << "\n";
        break;
    }
    case ACKNOWLEDGMENT:
    {
        break;
    }
    default:
        // Optional: handle unknown packet types
        break;
    }

    // Serialize and send ACK back
    std::vector<char> serialized_response = ackPacket.serialize();
    send(client_socket, serialized_response.data(), serialized_response.size(), 0);

    while (true)
    {

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
