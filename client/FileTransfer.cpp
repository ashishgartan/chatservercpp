#include "ChatClient.h"
#include "../packets/FilePacket.h"

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <algorithm>
// 🎨 Terminal Colors
#define RESET "\033[0m"
#define BLACK "\033[30m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"
#define WHITE "\033[37m"

// Now your send_file_by_chunks() and receive_data() implementations follow...

// Function to send a file by chunks using FilePackets
void ChatClient::send_file_by_chunks(const std::string &receiver, const std::string &full_path, int client_socket)
{
    FILE *file = fopen(full_path.c_str(), "rb");
    if (!file)
    {
        std::cout << RED << "❌ Error: Could not open file '" << full_path << "'\n"
                  << RESET;
        return;
    }

    fseek(file, 0, SEEK_END);
    size_t filesize = ftell(file);
    rewind(file);

    size_t chunk_size = 4096;                                     // Chunk size
    size_t num_chunks = (filesize + chunk_size - 1) / chunk_size; // Total chunks
    size_t bytes_sent = 0;

    for (size_t chunk_number = 0; chunk_number < num_chunks; ++chunk_number)
    {
        FilePacket packet;
        packet.sender = user_id;
        packet.receiver = receiver;
        packet.packet_number = chunk_number;
        packet.total_size = filesize;

        // Read the chunk data
        size_t bytes_to_read = std::min(chunk_size, filesize - chunk_number * chunk_size);
        char buffer[bytes_to_read];
        fread(buffer, 1, bytes_to_read, file);

        packet.data.assign(buffer, bytes_to_read); // Store chunk data in the packet

        // Serialize the packet before sending
        std::string serialized_data = packet.serialize();
        send(client_socket, serialized_data.c_str(), serialized_data.size(), 0);
        usleep(100000); // Simulate network delay

        bytes_sent += bytes_to_read;

        // 📈 Show percentage
        int percent = static_cast<int>((static_cast<double>(bytes_sent) / filesize) * 100);
        std::cout << GREEN << "\r📤 Sending: " << percent << "% complete." << RESET << std::flush;
    }

    fclose(file);
    std::cout << GREEN << "\n✅ File sent successfully." << RESET << "\n";
}

void ChatClient::receive_data(FilePacket filePacket)
{
    // Construct the filename
    std::string output_filename = "From_" + filePacket.sender + "_" + filePacket.filename;

    // Open the file in binary mode, appending if the file exists, creating it if it doesn't
    std::ofstream output_file(output_filename, std::ios::binary | std::ios::app);
    if (!output_file.is_open())
    {
        std::cerr << "❌ Failed to open file for writing: " << output_filename << "\n";
        return;
    }

    // Write the file chunk data
    output_file.write(filePacket.data.c_str(), filePacket.data.size());

    // Close the file
    output_file.close();

    std::cout << "✅ File received and saved as '" << output_filename << "' 🎉\n";
}