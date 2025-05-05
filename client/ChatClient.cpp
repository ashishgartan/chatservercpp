#include "ChatClient.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>
#include <sstream>
#include "../packets/PacketType.h"
#include "../packets/Packet.h"
#include "../packets/CommandType.h"

#define GREEN "\033[32m"
#define CYAN "\033[36m"
#define RED "\033[31m"
#define YELLOW "\033[33m"
#define RESET "\033[0m"

/**
 *
..............................................................
Command	        Query Code	    Example Usage
............................................................
/check [user]   0               /checkonline user123   done
/logout	        1	        	/logout                done
/history	    2	        	/history               done
/delete_account	3	            /delete_account        done
/online_users	4	            /online_users          done
/clear_chat	    5	            /clear_chat
/help       	6	            /help                  done
/whoami	        7	        	/whoami                done
/sendfile	    8	            /sendfile
/broadcast	    9	            /broadcast Hello everyone!   done
/block [user]	10	            /block user123
/unblock [user]	11	            /unblock user123
/profile	    12	            /profile
/changepass	    13	            /changepass newpassword123

 */
#define PORT 8080
#define MSG_SIZE 1024
ChatClient::ChatClient() : is_running(true)
{
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Connection failed");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    std::cout << "✅ Connected to server.\n";

    int choice;
    while (true)
    {
        std::cout << "1: Login\n2: Register\nEnter Your Choice:\n";
        std::cin >> choice;
        if (choice != 1 && choice != 2)
        {
            std::cout << "Invalid choice.\n";
            continue;
        }

        std::string user_id, password;

        std::cout << "Enter user ID: ";
        std::cin >> user_id;
        std::cout << "Enter password: ";
        std::cin >> password;

        // Create Packet for login/register request
        AeroProtocolPacket packet;
        packet.header.packetType = (choice == 1 ? LOGIN_REQUEST : REGISTER_REQUEST);
        packet.header.timestamp = std::time(nullptr);
        packet.username = user_id;
        packet.password = password;

        // std::cout << packet.toString();

        // Serialize and send the packet
        auto serialized_data = packet.serialize();

        auto printSerializedHex = [](const std::vector<char> &data)
        {
            std::cout << "📦 Serialized Packet (" << data.size() << " bytes):\n";
            for (size_t i = 0; i < data.size(); ++i)
            {
                printf("%02X ", static_cast<unsigned char>(data[i]));
                if ((i + 1) % 16 == 0)
                    std::cout << "\n";
            }
            std::cout << "\n";
        };

        printSerializedHex(serialized_data);

        int bytesend = send(client_socket, serialized_data.data(), serialized_data.size(), 0);
        std::cout << bytesend << " sent to server\n";
        // Receive ACK
        AeroProtocolHeader header;
        long unsigned int bytes_received = 0;
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

        // Step 2: Receive the payload based on the payload size in the header
        uint32_t payload_size = header.payloadSize;
        if (payload_size == 0)
        {
            std::cout << "No payload data to receive.\n";
            return; // No payload to process
        }

        // Receive the actual payload
        char *received_payload = new char[payload_size];
        unsigned int payload_received = 0;
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
        AeroProtocolPacket ackPacket;
        ackPacket.header = header;
        ackPacket = AeroProtocolPacket::deserialize(header, received_payload);
        delete[] received_payload; // Clean up the payload buffer after deserialization

        if (ackPacket.statusCode == 1)
        {
            std::cout << GREEN << packet.message << RESET;
            start();
        }
        else
        {
            std::cout << RED << packet.message << RESET;
            continue;
        }
    }
}

void ChatClient::start()
{
    send_thread = std::thread(&ChatClient::sendMessages, this);
    receive_thread = std::thread(&ChatClient::receiveMessages, this);
    send_thread.join();
    receive_thread.join();
}

void ChatClient::sendMessages()
{
    std::string message;
    std::cin.ignore(); // Clear leftover newline
    while (is_running)
    {
        std::cout << RED;
        std::getline(std::cin, message);
        std::cout << RESET;
        AeroProtocolPacket packet;
        std::vector<char> serialized_data;
        if (message.rfind("/checkOnline", 0) == 0)
        {
            std::istringstream iss(message);
            std::string cmd, user_id;
            iss >> cmd >> user_id;
            if (user_id.empty())
            {
                std::cout << RED << "❗ Usage: /check <user_id>\n"
                          << RESET;
                continue;
            }

            packet.senderId = user_id;
            packet.commandType = CHECK_ONLINE;
            packet.header.packetType = COMMAND;
            std::vector<char> serialized_packet = packet.serialize();
            send(client_socket, serialized_packet.data(), serialized_packet.size(), 0);
        }
        else if (message.rfind("/logout", 0) == 0)
        {
            packet.senderId = user_id;
            packet.header.packetType = COMMAND;
            packet.commandType = LOGOUT;
            std::vector<char> serialized_packet = packet.serialize();
            send(client_socket, serialized_packet.data(), serialized_packet.size(), 0);
        }
        else if (message.rfind("/history", 0) == 0)
        {
            std::istringstream iss(message);
            std::string cmd, user_id;
            iss >> cmd >> user_id;
            if (user_id.empty())
            {
                std::cout << RED << "❗ Usage: /history user_id>\n"
                          << RESET;
                continue;
            }

            packet.senderId = user_id;
            packet.header.packetType = COMMAND;
            packet.commandType = CHAT_HISTORY;
            std::vector<char> serialized_packet = packet.serialize();
            send(client_socket, serialized_packet.data(), serialized_packet.size(), 0);
        }
        else if (message.rfind("/delete_account", 0) == 0)
        {
            packet.senderId = user_id;
            packet.header.packetType = COMMAND;
            packet.commandType = DELETE_ACCOUNT;
            std::vector<char> serialized_packet = packet.serialize();
            send(client_socket, serialized_packet.data(), serialized_packet.size(), 0);
        }
        else if (message.rfind("/online_users", 0) == 0)
        {
            packet.senderId = user_id;
            packet.header.packetType = COMMAND;
            packet.commandType = ONLINE_USERS;
            std::vector<char> serialized_packet = packet.serialize();
            send(client_socket, serialized_packet.data(), serialized_packet.size(), 0);
        }
        else if (message.rfind("/clear_chat", 0) == 0)
        {
            std::istringstream iss(message);
            std::string cmd, receiver_id;
            iss >> cmd >> user_id;
            if (user_id.empty())
            {
                std::cout << RED << "❗ Usage: /clear_chat <user_id>\n"
                          << RESET;
                continue;
            }

            packet.senderId = user_id;
            packet.receiverId = receiver_id;
            packet.header.packetType = COMMAND;
            packet.commandType = CLEAR_CHAT;
            std::vector<char> serialized_packet = packet.serialize();
            send(client_socket, serialized_packet.data(), serialized_packet.size(), 0);
        }
        else if (message.rfind("/help", 0) == 0)
        {
            std::string helpMessage = "🛠 Available Commands:\n"
                                      "/logout                - Logout from chat\n"
                                      "/history               - View your chat history\n"
                                      "/online_users          - View online users\n"
                                      "/clear_chat            - Clear your chat\n"
                                      "/help                  - Show this help message\n"
                                      "/whoami                - See your username\n"
                                      "/broadcast [message]   - Send a message to all users\n"
                                      "/block [user]          - Block a user\n"
                                      "/unblock [user]        - Unblock a user\n";
            std::cout << helpMessage << std::endl;
        }
        else if (message.rfind("/whoami", 0) == 0)
        {
            std::cout << "I am " + user_id + "\n";
        }
        else if (message.rfind("/sendfile", 0) == 0)
        {
            std::istringstream iss(message);
            std::string cmd, receiver_id, filepath;
            iss >> cmd >> receiver_id >> filepath;
            if (receiver_id.empty() || filepath.empty())
            {
                std::cout << RED << "❗ Usage: /sendfile <receiver> <file_path>\n"
                          << RESET;
                continue;
            }

            std::thread file_thread(&ChatClient::send_file_by_chunks, this, receiver_id, filepath, client_socket);
            file_thread.detach();
        }
        else if (message.rfind("/broadcast", 0) == 0)
        {
            std::istringstream iss(message);
            std::string cmd, msg;
            iss >> cmd >> msg;
            if (user_id.empty())
            {
                std::cout << RED << "❗ Usage: /broadcast <msg>\n"
                          << RESET;
                continue;
            }

            packet.senderId = user_id;
            packet.header.packetType = COMMAND;
            packet.commandType = BROADCAST;
            packet.message = msg;
            std::vector<char> serialized_packet = packet.serialize();
            send(client_socket, serialized_packet.data(), serialized_packet.size(), 0);
        }
        else if (message.rfind("/block", 0) == 0)
        {
            std::istringstream iss(message);
            std::string cmd, receiver_id;
            iss >> cmd >> receiver_id;
            if (receiver_id.empty())
            {
                std::cout << RED << "❗ Usage: /block <user_id>\n"
                          << RESET;
                continue;
            }

            packet.senderId = user_id;
            packet.receiverId = receiver_id;
            packet.header.packetType = COMMAND;
            packet.commandType = BLOCK_USER;
            std::vector<char> serialized_packet = packet.serialize();
            send(client_socket, serialized_packet.data(), serialized_packet.size(), 0);
        }
        else if (message.rfind("/unblock", 0) == 0)
        {
            std::istringstream iss(message);
            std::string cmd, receiver_id;
            iss >> cmd >> receiver_id;
            if (receiver_id.empty())
            {
                std::cout << RED << "❗ Usage: /unblock <user_id>\n"
                          << RESET;
                continue;
            }

            packet.senderId = user_id;
            packet.receiverId = receiver_id;
            packet.header.packetType = COMMAND;
            packet.commandType = UNBLOCK_USER;
            std::vector<char> serialized_packet = packet.serialize();
            send(client_socket, serialized_packet.data(), serialized_packet.size(), 0);
        }
        else if (message.rfind("/profile", 0) == 0)
        {
            std::cout << "It will show profile, currently not implemented.";
        }
        else if (message.rfind("/changepass", 0) == 0)
        {
            std::istringstream iss(message);
            std::string cmd, new_pass, confirm_new_pass;
            iss >> cmd >> new_pass >> confirm_new_pass;
            if (new_pass.empty() || confirm_new_pass.empty())
            {
                std::cout << RED << "❗ Usage: /changepass new_pass confirm_new_pass\n"
                          << RESET;
                continue;
            }
            else if (new_pass != confirm_new_pass)
            {
                std::cout << RED << "❗ Password Mismatch.\n"
                          << RESET;
            }

            packet.senderId = user_id;
            packet.header.packetType = COMMAND;
            packet.commandType = CHANGE_PASSWORD;
            std::vector<char> serialized_packet = packet.serialize();
            send(client_socket, serialized_packet.data(), serialized_packet.size(), 0);
        }
        else
        {
            size_t space_pos = message.find(' ');
            if (space_pos != std::string::npos)
            {
                packet.senderId = user_id;
                packet.receiverId = message.substr(0, space_pos);
                packet.message = message.substr(space_pos + 1);
                serialized_data = packet.serialize();
                send(client_socket, serialized_data.data(), serialized_data.size(), 0);
            }

            if (message == "exit")
            {
                std::cout << "Disconnecting...\n";
                is_running = false;
                close(client_socket);
                exit(0);
            }
        }
    }
}
void ChatClient::receiveMessages()
{
    while (is_running)
    {
        // Receive ACK
        AeroProtocolHeader header;
        long unsigned int bytes_received = 0;
        while (bytes_received < sizeof(AeroProtocolHeader))
        {
            int chunk = recv(client_socket, ((char *)&header) + bytes_received, sizeof(AeroProtocolHeader) - bytes_received, 0);
            if (chunk <= 0)
            {
                std::cout << "\n🔌 Disconnected from server.\n";
                is_running = false;
                break;
            }
            bytes_received += chunk;
        }

        // Step 2: Receive the payload based on the payload size in the header
        uint32_t payload_size = header.payloadSize;
        if (payload_size == 0)
        {
            std::cout << "No payload data to receive.\n";
            return; // No payload to process
        }

        // Receive the actual payload
        char *received_payload = new char[payload_size];
        unsigned int payload_received = 0;
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
        packet = AeroProtocolPacket::deserialize(header, received_payload);
        delete[] received_payload; // Clean up the payload buffer after deserialization

        switch (packet.header.packetType)
        {
        case MESSAGE:
        {
            std::cout << packet.senderId << " : " << packet.message << RESET << std::endl;
        }
        case FILE_CHUNK:
        {
            receive_data(packet); // Adjust this line as needed
        }
        case ACKNOWLEDGMENT:
        {
            switch (packet.commandType)
            {
            case CHECK_ONLINE:
            {
                std::cout << packet.message << RESET << std::endl;
            }
            case LOGOUT:
            {
                std::cout << packet.message << RESET << std::endl;
                break;
            }
            case CHAT_HISTORY:
            {
                std::cout << packet.message << RESET << std::endl;
            }
            case DELETE_ACCOUNT:
            {
                std::cout << packet.message << RESET << std::endl;
            }
            case ONLINE_USERS:
            {
                std::cout << "Online Users:" << packet.message << RESET << std::endl;
            }
            case CLEAR_CHAT:
            {
                std::cout << packet.message << RESET << std::endl;
            }
            case SEND_FILE:
            {
                std::cout << packet.message << RESET << std::endl;
            }
            case BROADCAST:
            {
                std::cout << packet.message << RESET << std::endl;
            }
            case BLOCK_USER:
            {
            }
            case UNBLOCK_USER:
            {
            }
            case USER_PROFILE:
            {
            }
            case CHANGE_PASSWORD:
            {
            }
            default:
                // Optional: handle unknown command types
                break;
            }
        }
        default:
            // Optional: handle unknown packet types
            break;
        }
    }
}
ChatClient::~ChatClient()
{
    close(client_socket);
    std::cout << "🔚 Client shut down.\n";
}