#pragma once
enum PacketType
{
    LOGIN_REQUEST = 1,
    REGISTER_REQUEST,
    MESSAGE,
    FILE_CHUNK,
    AUDIO_CALL,
    VIDEO_CALL,
    ACKNOWLEDGMENT
};
