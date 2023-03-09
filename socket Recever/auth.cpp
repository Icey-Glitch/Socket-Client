#include "auth.h"
#include "messages.h"
#include "Packet.h"

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <openssl/evp.h>
#include <openssl/ssl.h>
#include <openssl/hmac.h>

#define MAX_PACKET_SIZE 1024

//#define AUTH_KEY "my_secret_key"


void auth::generateAuthTag(const char* message, unsigned char* authTag, size_t tagLen)
{
    unsigned char AUTH_KEY[] = "secret_key";
    EVP_PKEY* key = EVP_PKEY_new_mac_key(EVP_PKEY_HMAC, nullptr, AUTH_KEY, sizeof(AUTH_KEY) - 1);
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    const EVP_MD* md = EVP_sha256();

    EVP_DigestSignInit(ctx, NULL, md, NULL, key);
    EVP_DigestSignUpdate(ctx, reinterpret_cast<const unsigned char*>(message), strlen(message));
    EVP_DigestSignFinal(ctx, authTag, &tagLen);

    EVP_MD_CTX_reset(ctx);
    EVP_MD_CTX_free(ctx);
    EVP_PKEY_free(key);
}

int auth::verifyAuthTag(const char* message, const unsigned char* authTag)
{
    unsigned char computedTag[AUTH_TAG_SIZE];
    auth::generateAuthTag(message, computedTag, AUTH_TAG_SIZE);
    return memcmp(computedTag, authTag, AUTH_TAG_SIZE) == 0;
}

// Authenticate with the server using the given secret key.
int auth::authWithServer(int socket, const char* secretKey) {
    // Generate the authentication tag using HMAC-SHA256
    unsigned char authTag[auth::AUTH_TAG_SIZE];
    auth::generateAuthTag(secretKey, authTag, auth::AUTH_TAG_SIZE);

    // Send the authentication tag to the server
    int result = send(socket, reinterpret_cast<const char*>(authTag), auth::AUTH_TAG_SIZE, 0);
    if (result == SOCKET_ERROR) {
        printf("send failed: %d\n", WSAGetLastError());
        return 1;
    }

    // Receive the authentication result from the server
    int authResult;
    result = recv(socket, (char*)&authResult, sizeof(int), 0);
    if (result == SOCKET_ERROR) {
        printf("recv failed: %d\n", WSAGetLastError());
        return 1;
    }

    if (authResult == 0) {
        printf("Authentication failed. Invalid secret key.\n");
        return 1;
    } else if (authResult == 1) {
        printf("Authentication successful.\n");
        return 0;
    } else {
        printf("Unexpected authentication result received from server.\n");
        return 1;
    }
}

int auth::sendToServer(SOCKET socket, const char* message) {
    // Generate an authentication tag for the message
    unsigned char authTag[AUTH_TAG_SIZE];
    auth::generateAuthTag(message, authTag, AUTH_TAG_SIZE);

    // Serialize the message and authentication tag
    const int serializedLen = messages::serializedLen(strlen(message), AUTH_TAG_SIZE);
    messages::MessageType type = messages::MessageType::Request;
    char* serializedMsg = new char[serializedLen];
    messages::serialize(type, message, strlen(message), authTag, AUTH_TAG_SIZE);

    // Send the message to the server
    int bytesSent = send(socket, serializedMsg, serializedLen, 0);

    // Print the message and number of bytes sent
    std::cout << "Sent message: " << message << std::endl;
    std::cout << "Bytes sent: " << bytesSent << std::endl;

    delete[] serializedMsg;

    return bytesSent;
}

#pragma pack(push, 1) // Ensure struct is byte-aligned
struct PacketHeader {
    uint16_t packetSize;
    messages::MessageType messageType;
};
#pragma pack(pop)


bool auth::receiveFromServer(int socket) {
    // Receive the packet header from the server
    PacketHeader header;
    if (!Packet::receiveAll(socket, reinterpret_cast<char*>(&header), sizeof(header))) {
        printf("Failed to receive packet header.\n");
        return false;
    }

    // Check if the packet contains an authentication response
    if (header.messageType != messages::MessageType::AuthResponse) {
        printf("Received unexpected packet type from server.\n");
        return false;
    }

    // Receive the authentication response from the server
    int authResult;
    if (!Packet::receiveAll(socket, reinterpret_cast<char*>(&authResult), sizeof(authResult))) {
        printf("Failed to receive authentication response.\n");
        return false;
    }

    if (authResult == 0) {
        printf("Authentication failed. Invalid secret key.\n");
        return false;
    } else if (authResult == 1) {
        printf("Authentication successful.\n");
        return true;
    } else {
        printf("Unexpected authentication result received from server.\n");
        return false;
    }
}




