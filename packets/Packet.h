#pragma once
#include <vector>
#include <string>
#include <cstdint>
#include <cstring>
#include <arpa/inet.h>

#pragma pack(push, 1)
struct AeroProtocolHeader {
    uint32_t packetType = 0;       // Type of the packet (e.g., login, register, message)
    uint32_t payloadSize = 0;      // Size of the payload (excluding header)
    uint32_t sequenceNumber = 0;   // Sequence number for data integrity (if needed)
    uint64_t timestamp = 0;        // Timestamp when the packet is created
};
#pragma pack(pop)

struct AeroProtocolPacket {
    AeroProtocolHeader header={};
    std::vector<char> payload = {};

    // Fields based on packetType
    std::string senderId = "";      // User ID of the sender
    std::string receiverId = "";    // User ID of the receiver
    std::string message = "";       // Message content (e.g., text)
    uint32_t commandType = 0;      // Command type(e.g.,/sendfile,/changepass....)
    std::string username = "";      // For login or user-related packets
    std::string password = "";      // For login or authentication packets
    int32_t statusCode = 0;        // To represent success/failure numerically
    std::string fileName = "";      // File name (optional for file transfers)
    std::vector<char> data = {}; // data (optional for file transfers or any kind of data transfer)

    // Serialize and Deserialize methods
    std::vector<char> serialize();
    static AeroProtocolPacket deserialize(AeroProtocolHeader,const char *);

    std::string toString();
};

/** 
.....................................................................................
🔹 Core Fields (for all packet types) – already in AeroProtocolHeader
............................................................................
Field	                Purpose
packetType	            Identify the type: MESSAGE, LOGIN_REQUEST, FILE_CHUNK, etc.
payloadSize	            Total size of the payload
sequence                Number	Useful for chunked file transfers or packet reordering
timestamp	            When the packet was sent (epoch time)

...........................................................................................
🔹 Optional Payload-Level Fields (depend on packetType)
Below are common logical fields that can be encoded in the payload using length-prefixed strings:
...........................................................................................................
Field	                Used In PacketType	                Purpose
senderId (string)	    MESSAGE, FILE_CHUNK, ACK	        Who is sending the packet
receiverId (string)	    MESSAGE, FILE_CHUNK	                Who is receiving it
message (string)	    MESSAGE	                            Actual text message
username (string)	    LOGIN_REQUEST, REGISTER_REQUEST	    For login/register
password (string)	    LOGIN_REQUEST, REGISTER_REQUEST	    For login/register
filename (string)	    FILE_CHUNK	                        Name of the file being transferred
fileData (binary blob)	FILE_CHUNK	                        Actual chunk of the file
ackMessage (string)	    ACKNOWLEDGMENT	                    Optional status message like "OK", "Failed", etc.
statusCode (int32)	    ACKNOWLEDGMENT	                    To represent success/failure numerically


.......................................
Example: Packet Types and Fields
...................................
PacketType	        Required Fields
LOGIN_REQUEST	    username, password
REGISTER_REQUEST	username, password
MESSAGE	            senderId, receiverId, message
ACKNOWLEDGMENT	    senderId, ackMessage, statusCode
FILE_CHUNK	        senderId, receiverId, filename, fileData, sequenceNumber


enum PacketType {
    LOGIN_REQUEST = 1,
    REGISTER_REQUEST,
    MESSAGE,
    FILE_CHUNK,
    AUDIO_CALL,
    VIDEO_CALL,
    ACKNOWLEDGMENT
};

*/
