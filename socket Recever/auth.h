#pragma once
#include <winsock2.h>
#include <openssl/types.h>

class auth
{
public:
    static constexpr unsigned int AUTH_KEY_SIZE = 32;
    static constexpr unsigned int AUTH_TAG_SIZE = 32;
    static constexpr unsigned int AUTH_MESSAGE_SIZE = 1024;
    static void generateAuthTag(const char* message, unsigned char* authTag, size_t tagLen);
    //static void generateAuthTag(const char* message, unsigned char* authTag);
    static int verifyAuthTag(const char* message, const unsigned char* authTag);
    static int authWithServer(int socket, const char* secretKey);
    static int sendToServer(SOCKET socket, const char* message);
    //static bool receiveFromServer(char*& buffer, int& bufferLen);
    static bool receiveFromServer(int socket);
};
