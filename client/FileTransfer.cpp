#include "ChatClient.h"
#include "../packets/FilePacket.h"
#include <filesystem>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <algorithm>
#include <vector>
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
    std::filesystem::path file_path(full_path);
    std::string filename = file_path.filename().string();
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

    // The header (packetType|queryType) can take up to 100 bytes (adjust as needed)
    size_t header_size = 8;
    size_t chunk_size = 2048;
    // Calculate the available space for the data (taking into account the header size)
    size_t available_data_size = chunk_size - header_size;                                                  // Chunk size
    size_t chunks_count = (filesize / available_data_size) + (filesize % available_data_size == 0 ? 0 : 1); // Total chunks
    size_t bytes_sent = 0;
    std::cout << "📦 Number of chunks to send: " << chunks_count << "\n";
    FilePacket filePacket;
    filePacket.sender = user_id;
    filePacket.receiver = receiver;
    filePacket.total_size = filesize;
    filePacket.filename = filename;
    for (size_t chunk_number = 0; chunk_number < chunks_count; ++chunk_number)
    {
        filePacket.packet_number = chunk_number;

        // Read the chunk data
        size_t bytes_to_read = std::min(chunk_size, filesize - chunk_number * chunk_size);
        std::vector<char> buffer(bytes_to_read);
        fread(buffer.data(), 1, bytes_to_read, file);
        filePacket.data.assign(buffer.begin(), buffer.end());

        // Serialize the filePacket before sending
        std::string serialized_data = filePacket.serialize();
        send(client_socket, serialized_data.c_str(), serialized_data.size(), 0);
        usleep(100000); // Simulate network delay

        bytes_sent += bytes_to_read;
        std::cout << GREEN << "\r📤 Sending: Packet No:" << chunk_number+1 << RESET;
        // 📈 Show percentage
        // int percent = static_cast<int>((static_cast<double>(bytes_sent) / filesize) * 100);
        // std::cout << GREEN << "\r📤 Sending: " << percent << "% complete." << RESET << std::flush;
    }

    fclose(file);
    std::cout << GREEN << "\n✅ File sent successfully." << RESET << "\n";
    return;
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