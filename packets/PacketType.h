#pragma once
enum PacketType
{
    LOGIN_REQUEST = 1,
    REGISTER_REQUEST = 2,
    MESSAGE =3 ,
    COMMAND = 4,
    FILE_CHUNK = 5,
    AUDIO_CALL = 6,
    VIDEO_CALL = 7,
    ACKNOWLEDGMENT = 8
};
