#include "Packet.h"
#include "PacketType.h"
#include <cstring> // for memcpy
#include <iostream>
#include <sstream>
#include <iomanip> // for formatting

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
        serializedPayload.insert(serializedPayload.end(), fileData.begin(), fileData.end());
        break;
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
        uint32_t ackLen = htonl(ackMessage.size());
        serializedPayload.insert(serializedPayload.end(), reinterpret_cast<char *>(&ackLen), reinterpret_cast<char *>(&ackLen) + sizeof(ackLen));
        serializedPayload.insert(serializedPayload.end(), ackMessage.begin(), ackMessage.end());
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

    // 🎯 Convert all header fields from network byte order to host byte order
    pkt.header.packetType = ntohl(networkHeader.packetType);
    pkt.header.payloadSize = ntohl(networkHeader.payloadSize);
    pkt.header.sequenceNumber = ntohl(networkHeader.sequenceNumber);
    pkt.header.timestamp = be64toh(networkHeader.timestamp); // 64-bit big endian to host

    // 📦 Resize payload container to hold the actual data
    pkt.payload.resize(pkt.header.payloadSize);

    // 🧠 Copy raw payload bytes into the vector
    std::memcpy(pkt.payload.data(), received_payload, pkt.payload.size());

    // 🔍 Begin parsing the payload
    const char *ptr = pkt.payload.data();

    // 🧵 Handle packet types based on the value
    switch (pkt.header.packetType)
    {
    case MESSAGE:
    {
        // 🧱 Step 1: Read sender ID length (4 bytes)
        uint32_t senderLen;
        std::memcpy(&senderLen, ptr, sizeof(senderLen));
        senderLen = ntohl(senderLen);
        ptr += sizeof(senderLen); // ⏭️ Move past the length field

        // 📛 Step 2: Read sender ID string
        std::string senderId(ptr, senderLen);
        ptr += senderLen; // ⏭️ Move past the sender string

        // 🧱 Step 3: Read receiver ID length (4 bytes)
        uint32_t receiverLen;
        std::memcpy(&receiverLen, ptr, sizeof(receiverLen));
        receiverLen = ntohl(receiverLen);
        ptr += sizeof(receiverLen);

        // 📛 Step 4: Read receiver ID string
        std::string receiverId(ptr, receiverLen);
        ptr += receiverLen;

        // 🧱 Step 5: Read message length (4 bytes)
        uint32_t msgLen;
        std::memcpy(&msgLen, ptr, sizeof(msgLen));
        msgLen = ntohl(msgLen);
        ptr += sizeof(msgLen);

        // 💬 Step 6: Read actual message
        std::string message(ptr, msgLen);

        // 🖨️ Log it
        std::cout << "[💬 MESSAGE] From: " << senderId << ", To: " << receiverId << ", Msg: " << message << "\n";
        break;
    }

    case ACKNOWLEDGMENT:
    {
        // ✅ Step 1: Read ack message length
        uint32_t ackLen;
        std::memcpy(&ackLen, ptr, sizeof(ackLen));
        ackLen = ntohl(ackLen);
        ptr += sizeof(ackLen);

        // 📨 Step 2: Read actual acknowledgment message
        std::string ackMessage(ptr, ackLen);

        // 🖨️ Log it
        std::cout << "[📨 ACK] Message: " << ackMessage << "\n";
        break;
    }

    case LOGIN_REQUEST:
    {
        // 👤 Step 1: Read username length
        uint32_t userLen;
        std::memcpy(&userLen, ptr, sizeof(userLen));
        userLen = ntohl(userLen);
        ptr += sizeof(userLen);

        // 👤 Step 2: Read username
        std::string username(ptr, userLen);
        ptr += userLen;

        // 🔐 Step 3: Read password length
        uint32_t passLen;
        std::memcpy(&passLen, ptr, sizeof(passLen));
        passLen = ntohl(passLen);
        ptr += sizeof(passLen);

        // 🔐 Step 4: Read password
        std::string password(ptr, passLen);

        // 🖨️ Log it
        std::cout << "[🔐 LOGIN] Username: " << username << ", Password: " << password << "\n";
        break;
    }

    // 🧩 TODO: Add more packet types like REGISTER, FILE, LOGOUT, etc.
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
        oss << "File Size: " << fileData.size() << " bytes\n";
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
        oss << "Message: " << ackMessage << "\n";
        break;

    default:
        oss << "[UNKNOWN TYPE]\n";
        break;
    }

    return oss.str();
}