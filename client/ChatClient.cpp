#include "ChatClient.h"
// #include "FileTransfer.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>
#include <sstream>
#include "../packets/QueryPacket.h"
#include "../packets/LoginSignupPacket.h"
#include "../packets/ChatMessagePacket.h"
#include "../packets/FilePacket.h"
#include "../packets/ACKPacket.h"
#include "../packets/PacketType.h"

#define GREEN "\033[32m"
#define CYAN "\033[36m"
#define RED "\033[31m"
#define YELLOW "\033[33m"
#define RESET "\033[0m"
/*
    LOGIN_PACKET = 1,
    REGISTER_PACKET = 2,
    CHAT_MESSAGE_PACKET = 3,
    FILE_PACKET = 4,
    ACK_PACKET = 5,
    QUERY_PACKET = 6
*/

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
    std::cout << "✅ Client socket created successfully.\n";

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Connection to server failed");
        close(client_socket);
        exit(EXIT_FAILURE);
    }
    std::cout << "🔗 Connected to server.\n";

    int choice;
    while (true)
    {
        std::cout << "1: Login\n2: Register\nEnter Your Choice:\n";
        std::cin >> choice;
        if (choice != 1 && choice != 2)
        {
            std::cout << "Invalid Choice.🫩 \n";
            continue;
        }

        int type = (choice == 1 ? 1 : 2);
        std::string user_id;
        std::string password;

        std::cout << "\nEnter user_id number: ";
        std::cin >> user_id;
        std::cout << "Enter password: ";
        std::cin >> password;

        // Create and send login/signup packet
        LoginSignupPacket lsp(user_id, password);
        lsp.packetType = type;
        std::string serialized_packet = lsp.serialize();
        send(client_socket, serialized_packet.c_str(), serialized_packet.size(), 0);

        // Receive ACK
        ACKPacket ackpacket;
        char ack_buffer[1024] = {0};
        recv(client_socket, ack_buffer, sizeof(ack_buffer), 0);
        ackpacket.deserialize(ack_buffer);
        if (ackpacket.status == 1)
        {
            std::cout << GREEN "Success: " << ackpacket.message << RESET << std::endl;
            this->user_id = user_id; // Save user id for message sending
            this->start();
            break;
        }
        else
        {
            std::cout << RED "Failure: " << ackpacket.message << RESET << std::endl;
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
        QueryPacket queryPacket;
        std::string serialized_packet;
        if (message.rfind("/check", 0) == 0)
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

            queryPacket.data = user_id;
            queryPacket.queryType = 0;
            serialized_packet = queryPacket.serialize();
            send(client_socket, serialized_packet.c_str(), serialized_packet.size(), 0);
        }
        else if (message.rfind("/logout", 0) == 0)
        {
            queryPacket.data = user_id;
            queryPacket.queryType = 1;
            serialized_packet = queryPacket.serialize();
            send(client_socket, serialized_packet.c_str(), serialized_packet.size(), 0);
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

            queryPacket.data = user_id;
            queryPacket.queryType = 2;
            serialized_packet = queryPacket.serialize();
            send(client_socket, serialized_packet.c_str(), serialized_packet.size(), 0);
        }
        else if (message.rfind("/delete_account", 0) == 0)
        {
            queryPacket.queryType = 3;
            serialized_packet = queryPacket.serialize();
            send(client_socket, serialized_packet.c_str(), serialized_packet.size(), 0);
        }
        else if (message.rfind("/online_users", 0) == 0)
        {
            queryPacket.queryType = 4;
            serialized_packet = queryPacket.serialize();
            // std::cout << serialized_packet << std::endl;
            send(client_socket, serialized_packet.c_str(), serialized_packet.size(), 0);
        }
        else if (message.rfind("/clear_chat", 0) == 0)
        {
            std::istringstream iss(message);
            std::string cmd, user_id;
            iss >> cmd >> user_id;
            if (user_id.empty())
            {
                std::cout << RED << "❗ Usage: /clear_chat <user_id>\n"
                          << RESET;
                continue;
            }

            queryPacket.data = user_id;
            queryPacket.queryType = 5;
            serialized_packet = queryPacket.serialize();
            send(client_socket, serialized_packet.c_str(), serialized_packet.size(), 0);
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
            std::string cmd, receiver, filepath;
            iss >> cmd >> receiver >> filepath;
            if (receiver.empty() || filepath.empty())
            {
                std::cout << RED << "❗ Usage: /sendfile <receiver> <file_path>\n"
                          << RESET;
                continue;
            }

            std::thread file_thread(&ChatClient::send_file_by_chunks, this, receiver, filepath, client_socket);
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

            queryPacket.data = msg;
            queryPacket.queryType = 9;
            serialized_packet = queryPacket.serialize();
            send(client_socket, serialized_packet.c_str(), serialized_packet.size(), 0);
        }
        else if (message.rfind("/block", 0) == 0)
        {
            std::istringstream iss(message);
            std::string cmd, user_id;
            iss >> cmd >> user_id;
            if (user_id.empty())
            {
                std::cout << RED << "❗ Usage: /block <user_id>\n"
                          << RESET;
                continue;
            }

            queryPacket.data = user_id;
            queryPacket.queryType = 10;
            serialized_packet = queryPacket.serialize();
            send(client_socket, serialized_packet.c_str(), serialized_packet.size(), 0);
        }
        else if (message.rfind("/unblock", 0) == 0)
        {
            std::istringstream iss(message);
            std::string cmd, user_id;
            iss >> cmd >> user_id;
            if (user_id.empty())
            {
                std::cout << RED << "❗ Usage: /unblock <user_id>\n"
                          << RESET;
                continue;
            }

            queryPacket.data = user_id;
            queryPacket.queryType = 11;
            serialized_packet = queryPacket.serialize();
            send(client_socket, serialized_packet.c_str(), serialized_packet.size(), 0);
        }
        else if (message.rfind("/profile", 0) == 0)
        {
            std::cout << "It will show profile, currently not implemented.";
        }
        else if (message.rfind("/changepass", 0) == 0)
        {
            std::istringstream iss(message);
            std::string cmd, receiver, filepath;
            iss >> cmd >> receiver >> filepath;
            if (receiver.empty() || filepath.empty())
            {
                std::cout << RED << "❗ Usage: /sendData <receiver> <file_path>\n"
                          << RESET;
                continue;
            }

            std::thread file_thread(&ChatClient::send_file_by_chunks, this, receiver, filepath, client_socket);
            file_thread.detach();
        }
        else
        {
            size_t space_pos = message.find(' ');
            if (space_pos != std::string::npos)
            {
                ChatMessagePacket chatPacket;
                chatPacket.sender = user_id;
                chatPacket.receiver = message.substr(0, space_pos);
                chatPacket.message = message.substr(space_pos + 1);
                serialized_packet = chatPacket.serialize();
                send(client_socket, serialized_packet.c_str(), serialized_packet.size(), 0);
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
        char received_serialized_data[4096] = {0};
        int bytes_read = recv(client_socket, received_serialized_data, sizeof(received_serialized_data), 0);
        if (bytes_read <= 0)
        {
            std::cout << "\n🔌 Disconnected from server.\n";
            is_running = false;
            break;
        }

        // Extract the packet type from the received data
        std::string data_str(received_serialized_data);
        int type = 0;
        size_t pos = data_str.find('|'); // Assuming packetType is the first part of the string
        type = std::stoi(data_str.substr(0, pos));
        std::string queryresponse = "";
        if (type == 3)
        {
            ChatMessagePacket chatPacket;
            chatPacket.deserialize(received_serialized_data);
            std::cout << GREEN << chatPacket.sender << ": " << chatPacket.message << RESET << std::endl;
        }
        else if (type == 4)
        {
            FilePacket filePacket;
            filePacket.deserialize(received_serialized_data);
            receive_data(filePacket); // Adjust this line as needed
        }
        else if (type == 6)
        {
            QueryPacket queryPacket;
            queryPacket.deserialize(received_serialized_data);

            if (queryPacket.queryType == 2)
            {
                // Check if the received data contains the emoji for a new line
                if (queryPacket.data.find("↩️") != std::string::npos)
                {
                    // Replace the emoji with a newline character and continue printing
                    size_t pos = queryPacket.data.find("↩️");
                    while (pos != std::string::npos)
                    {
                        queryPacket.data.replace(pos, 2, "\n");
                        pos = queryPacket.data.find("↩️", pos + 1);
                    }
                }

                std::cout << GREEN << queryPacket.data << RESET << std::endl;
            }
            else if (queryPacket.queryType == 3)
            {
                if(queryPacket.data == "1"){
                    std::cout <<GREEN<<"✅ Account deleted successfully." << RESET<<std::endl;
                }
                else{
                    std::cout <<RED<<"❌ Error deleting account" << RESET<<std::endl;
                }
            }
            else if(queryPacket.queryType == 4){
                std::cout<<GREEN<<queryPacket.data<<RESET<<std::endl;
            }
        }
        else
        {
            std::cout << YELLOW << "⚠️ Unknown packet type received.\n"
                      << RESET;
        }
    }
    exit(0);
}

ChatClient::~ChatClient()
{
    close(client_socket);
    std::cout << "🔚 Client shut down.\n";
}