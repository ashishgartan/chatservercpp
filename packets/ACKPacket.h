#pragma once
#include <string>

class ACKPacket
{
public:
    int packetType = 5;
    int status;
    int error_code;
    std::string message;

    ACKPacket(int s = 0, const std::string &msg = "");

    std::string serialize() const;
    void deserialize(const std::string &str);
};
/*
Field      |Meaning                                        | Possible Values
________________________________________________________________________________________________________
packetType | Identifies this packet as an ACK              | Always 5
status     | General success or failure                    | 0 = Failure, 1 = Success (this is common)
error_code | Specific reason for failure (or 0 if success) | E.g., 0 = No Error,
                                                                   1 = User Not Found,
                                                                   2 = Wrong Password,
                                                                   3 = Already Registered,
                                                                   etc. (you can define these yourself)
message    |Text message for human-readable explanation   | Examples: "Login successful",
                                                                      "Registration failed: User already exists",
                                                                      "Server Error"
*/
