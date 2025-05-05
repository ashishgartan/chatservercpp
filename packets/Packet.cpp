#include "Packet.h"
#include "PacketType.h"
#include <cstring> // for memcpy
#include <iostream>
#include <sstream>
#include <iomanip> // for formatting
#define GREEN "\033[32m"
#define CYAN "\033[36m"
#define RED "\033[31m"
#define YELLOW "\033[33m"
#define RESET "\033[0m"
// Fallback for systems missing htobe64
// uint64_t htobe64(uint64_t val)
// {
//     uint32_t high = htonl(static_cast<uint32_t>(val >> 32));
//     uint32_t low = htonl(static_cast<uint32_t>(val & 0xFFFFFFFF));
//     return (static_cast<uint64_t>(low) << 32) | high;
// }

// uint64_t be64toh(uint64_t val)
// {
//     uint32_t high = ntohl(static_cast<uint32_t>(val & 0xFFFFFFFF));
//     uint32_t low = ntohl(static_cast<uint32_t>(val >> 32));
//     return (static_cast<uint64_t>(high) << 32) | low;
// }

// Serialize the AeroProtocolPacket into a byte array
std::vector<char> AeroProtocolPacket::serialize()
{
    std::vector<char> serializedPayload;

    switch (header.packetType)
    {
    case LOGIN_REQUEST:
    case REGISTER_REQUEST:
    {
        uint32_t userLen = htonl(username.size());
        serializedPayload.insert(serializedPayload.end(), reinterpret_cast<char *>(&userLen), reinterpret_cast<char *>(&userLen) + sizeof(userLen));
        serializedPayload.insert(serializedPayload.end(), username.begin(), username.end());

        uint32_t passLen = htonl(password.size());
        serializedPayload.insert(serializedPayload.end(), reinterpret_cast<char *>(&passLen), reinterpret_cast<char *>(&passLen) + sizeof(passLen));
        serializedPayload.insert(serializedPayload.end(), password.begin(), password.end());
        break;
    }

    case MESSAGE:
    {
        uint32_t senderLen = htonl(senderId.size());
        serializedPayload.insert(serializedPayload.end(), reinterpret_cast<char *>(&senderLen), reinterpret_cast<char *>(&senderLen) + sizeof(senderLen));
        serializedPayload.insert(serializedPayload.end(), senderId.begin(), senderId.end());

        uint32_t receiverLen = htonl(receiverId.size());
        serializedPayload.insert(serializedPayload.end(), reinterpret_cast<char *>(&receiverLen), reinterpret_cast<char *>(&receiverLen) + sizeof(receiverLen));
        serializedPayload.insert(serializedPayload.end(), receiverId.begin(), receiverId.end());

        uint32_t messageLen = htonl(message.size());
        serializedPayload.insert(serializedPayload.end(), reinterpret_cast<char *>(&messageLen), reinterpret_cast<char *>(&messageLen) + sizeof(messageLen));
        serializedPayload.insert(serializedPayload.end(), message.begin(), message.end());
        break;
    }

    case FILE_CHUNK:
    {
        uint32_t senderLen = htonl(senderId.size());
        serializedPayload.insert(serializedPayload.end(), reinterpret_cast<char *>(&senderLen), reinterpret_cast<char *>(&senderLen) + sizeof(senderLen));
        serializedPayload.insert(serializedPayload.end(), senderId.begin(), senderId.end());

        uint32_t receiverLen = htonl(receiverId.size());
        serializedPayload.insert(serializedPayload.end(), reinterpret_cast<char *>(&receiverLen), reinterpret_cast<char *>(&receiverLen) + sizeof(receiverLen));
        serializedPayload.insert(serializedPayload.end(), receiverId.begin(), receiverId.end());

        uint32_t fileNameLen = htonl(fileName.size());
        serializedPayload.insert(serializedPayload.end(), reinterpret_cast<char *>(&fileNameLen), reinterpret_cast<char *>(&fileNameLen) + sizeof(fileNameLen));
        serializedPayload.insert(serializedPayload.end(), fileName.begin(), fileName.end());

        // Add raw file data
        serializedPayload.insert(serializedPayload.end(), data.begin(), data.end());
        break;
    }
    case COMMAND:
    {
        uint32_t commandTypeLen = htonl(sizeof(commandType));
        serializedPayload.insert(serializedPayload.end(), reinterpret_cast<char *>(&commandTypeLen), reinterpret_cast<char *>(&commandTypeLen) + sizeof(commandTypeLen));
        serializedPayload.insert(serializedPayload.end(), reinterpret_cast<char *>(&commandType), reinterpret_cast<char *>(&commandType) + sizeof(commandType));

        // uint32_t messageLen = htonl(message.size());
        // serializedPayload.insert(serializedPayload.end(), reinterpret_cast<char *>(&messageLen), reinterpret_cast<char *>(&messageLen) + sizeof(messageLen));
        // serializedPayload.insert(serializedPayload.end(), message.begin(), message.end());

        // uint32_t messageLen = htonl(message.size());
        // serializedPayload.insert(serializedPayload.end(), reinterpret_cast<char *>(&messageLen), reinterpret_cast<char *>(&messageLen) + sizeof(messageLen));
        // serializedPayload.insert(serializedPayload.end(), message.begin(), message.end());
    }
        // case AUDIO_CALL:
        // case VIDEO_CALL:
        // {
        //     uint32_t senderLen = htonl(senderId.size());
        //     serializedPayload.insert(serializedPayload.end(), reinterpret_cast<char *>(&senderLen), reinterpret_cast<char *>(&senderLen) + sizeof(senderLen));
        //     serializedPayload.insert(serializedPayload.end(), senderId.begin(), senderId.end());
        //     uint32_t receiverLen = htonl(receiverId.size());
        //     serializedPayload.insert(serializedPayload.end(), reinterpret_cast<char *>(&receiverLen), reinterpret_cast<char *>(&receiverLen) + sizeof(receiverLen));
        //     serializedPayload.insert(serializedPayload.end(), receiverId.begin(), receiverId.end());
        //     uint32_t callDataLen = htonl(callMetadata.size());
        //     serializedPayload.insert(serializedPayload.end(), reinterpret_cast<char *>(&callDataLen), reinterpret_cast<char *>(&callDataLen) + sizeof(callDataLen));
        //     serializedPayload.insert(serializedPayload.end(), callMetadata.begin(), callMetadata.end());
        //     break;
        // }

    case ACKNOWLEDGMENT:
    {
        uint32_t ackLen = htonl(message.size());
        serializedPayload.insert(serializedPayload.end(), reinterpret_cast<char *>(&ackLen), reinterpret_cast<char *>(&ackLen) + sizeof(ackLen));
        serializedPayload.insert(serializedPayload.end(), message.begin(), message.end());
        break;
    }

    default:
        // Optional: handle unknown packet types
        break;
    }

    payload = serializedPayload;
    header.payloadSize = payload.size();

    AeroProtocolHeader networkHeader = header;
    networkHeader.packetType = htonl(header.packetType);
    networkHeader.payloadSize = htonl(header.payloadSize);
    networkHeader.sequenceNumber = htonl(header.sequenceNumber);
    networkHeader.timestamp = htobe64(header.timestamp);

    std::vector<char> data(sizeof(AeroProtocolHeader) + payload.size());
    std::memcpy(data.data(), &networkHeader, sizeof(AeroProtocolHeader));
    std::memcpy(data.data() + sizeof(AeroProtocolHeader), payload.data(), payload.size());

    return data;
}

// Deserialize a byte array into an AeroProtocolPacket
AeroProtocolPacket AeroProtocolPacket::deserialize(AeroProtocolHeader networkHeader, const char *received_payload)
{
    AeroProtocolPacket pkt;
    pkt.header = networkHeader;


    // 📦 Resize payload container to hold the actual data
    pkt.payload.resize(pkt.header.payloadSize);

    // 🧠 Copy raw payload bytes into the vector
    std::memcpy(pkt.payload.data(), received_payload, pkt.payload.size());

    // 🔍 Begin parsing the payload
    const char *ptr = pkt.payload.data();

    const char *payloadEnd = pkt.payload.data() + pkt.payload.size();

    switch (pkt.header.packetType)
    {
    case MESSAGE:
    {
        uint32_t senderLen;
        if (ptr + sizeof(senderLen) > payloadEnd)
        {
            std::cerr << RED << "❌ MESSAGE: Incomplete sender length.\n"
                      << RESET;
            return pkt;
        }
        std::memcpy(&senderLen, ptr, sizeof(senderLen));
        senderLen = ntohl(senderLen);
        ptr += sizeof(senderLen);

        if (ptr + senderLen > payloadEnd)
        {
            std::cerr << RED << "❌ MESSAGE: Incomplete sender string.\n"
                      << RESET;
            return pkt;
        }
        pkt.senderId = std::string(ptr, senderLen);
        ptr += senderLen;

        uint32_t receiverLen;
        if (ptr + sizeof(receiverLen) > payloadEnd)
        {
            std::cerr << RED << "❌ MESSAGE: Incomplete receiver length.\n"
                      << RESET;
            return pkt;
        }
        std::memcpy(&receiverLen, ptr, sizeof(receiverLen));
        receiverLen = ntohl(receiverLen);
        ptr += sizeof(receiverLen);

        if (ptr + receiverLen > payloadEnd)
        {
            std::cerr << RED << "❌ MESSAGE: Incomplete receiver string.\n"
                      << RESET;
            return pkt;
        }
        pkt.receiverId = std::string(ptr, receiverLen);
        ptr += receiverLen;

        uint32_t msgLen;
        if (ptr + sizeof(msgLen) > payloadEnd)
        {
            std::cerr << RED << "❌ MESSAGE: Incomplete message length.\n"
                      << RESET;
            return pkt;
        }
        std::memcpy(&msgLen, ptr, sizeof(msgLen));
        msgLen = ntohl(msgLen);
        ptr += sizeof(msgLen);

        if (ptr + msgLen > payloadEnd)
        {
            std::cerr << RED << "❌ MESSAGE: Incomplete message data.\n"
                      << RESET;
            return pkt;
        }
        pkt.message = std::string(ptr, msgLen);
        break;
    }

    case ACKNOWLEDGMENT:
    {
        uint32_t ackLen;
        if (ptr + sizeof(ackLen) > payloadEnd)
        {
            std::cerr << RED << "❌ ACK: Incomplete ack length.\n"
                      << RESET;
            return pkt;
        }
        std::memcpy(&ackLen, ptr, sizeof(ackLen));
        ackLen = ntohl(ackLen);
        ptr += sizeof(ackLen);

        if (ptr + ackLen > payloadEnd)
        {
            std::cerr << RED << "❌ ACK: Incomplete ack message.\n"
                      << RESET;
            return pkt;
        }
        pkt.message = std::string(ptr, ackLen);
        break;
    }

    case REGISTER_REQUEST:
    case LOGIN_REQUEST:
    {
        uint32_t userLen;
        if (ptr + sizeof(userLen) > payloadEnd)
        {
            std::cerr << RED << "❌ LOGIN/REGISTER: Incomplete username length.\n"
                      << RESET;
            return pkt;
        }
        std::memcpy(&userLen, ptr, sizeof(userLen));
        userLen = ntohl(userLen);
        ptr += sizeof(userLen);

        if (ptr + userLen > payloadEnd)
        {
            std::cerr << RED << "❌ LOGIN/REGISTER: Incomplete username.\n"
                      << RESET;
            return pkt;
        }
        pkt.username = std::string(ptr, userLen);
        ptr += userLen;

        uint32_t passLen;
        if (ptr + sizeof(passLen) > payloadEnd)
        {
            std::cerr << RED << "❌ LOGIN/REGISTER: Incomplete password length.\n"
                      << RESET;
            return pkt;
        }
        std::memcpy(&passLen, ptr, sizeof(passLen));
        passLen = ntohl(passLen);
        ptr += sizeof(passLen);

        if (ptr + passLen > payloadEnd)
        {
            std::cerr << RED << "❌ LOGIN/REGISTER: Incomplete password.\n"
                      << RESET;
            return pkt;
        }
        pkt.password = std::string(ptr, passLen);
        break;
    }

    case FILE_CHUNK:
    {
        std::cerr << YELLOW << "⚠️ FILE_CHUNK: Deserialization not yet implemented.\n"
                  << RESET;
        break;
    }
    case COMMAND:
    {
        std::cerr << YELLOW << "⚠️ COMMAND: Deserialization not yet implemented.\n"
                  << RESET;
        break;
    }

    default:
        std::cerr << RED << "❌ Unknown packet type: " << pkt.header.packetType << "\n"
                  << RESET;
        break;
    }

    // ✅ Return the fully parsed packet
    return pkt;
}
std::string AeroProtocolPacket::toString()
{
    std::ostringstream oss;
    oss << "=== AeroProtocol Packet ===\n";
    oss << "Type: " << header.packetType << "\n";
    oss << "Seq #: " << header.sequenceNumber << "\n";
    oss << "Timestamp: " << header.timestamp << "\n";
    oss << "Payload Size: " << header.payloadSize << "\n";

    switch (header.packetType)
    {
    case LOGIN_REQUEST:
        oss << "[LOGIN_REQUEST]\n";
        oss << "Username: " << username << "\n";
        oss << "Password: " << password << "\n";
        break;

    case REGISTER_REQUEST:
        oss << "[REGISTER_REQUEST]\n";
        oss << "Username: " << username << "\n";
        oss << "Password: " << password << "\n";
        break;

    case MESSAGE:
        oss << "[MESSAGE]\n";
        oss << "From: " << senderId << "\n";
        oss << "To: " << receiverId << "\n";
        oss << "Message: " << message << "\n";
        break;

    case FILE_CHUNK:
        oss << "[FILE_CHUNK]\n";
        oss << "From: " << senderId << "\n";
        oss << "To: " << receiverId << "\n";
        oss << "File Name: " << fileName << "\n";
        oss << "File Size: " << data.size() << " bytes\n";
        break;

        // case AUDIO_CALL:
        //     oss << "[AUDIO_CALL]\n";
        //     oss << "From: " << senderId << "\n";
        //     oss << "To: " << receiverId << "\n";
        //     oss << "Call Metadata: " << callMetadata << "\n";
        //     break;

        // case VIDEO_CALL:
        //     oss << "[VIDEO_CALL]\n";
        //     oss << "From: " << senderId << "\n";
        //     oss << "To: " << receiverId << "\n";
        //     oss << "Call Metadata: " << callMetadata << "\n";
        //     break;

    case ACKNOWLEDGMENT:
        oss << "[ACKNOWLEDGMENT]\n";
        oss << "Message: " << message << "\n";
        break;

    default:
        oss << "[UNKNOWN TYPE]\n";
        break;
    }

    return oss.str();
}