#include "messages.h"

#include <algorithm>
#include <string>
#include <cstring>
#include <iostream>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <winsock2.h>
#include "messages.h"
#include <sstream>
#include <vector>

std::vector<std::string> splitString(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

bool verifyAuthKey(const std::string& message, const std::string& authKey) {
    // Split the message into its components
    std::vector<std::string> parts = splitString(message, '|');

    // Verify that the message has at least two parts
    if (parts.size() < 2) {
        return false;
    }

    // Extract the received auth key
    std::string receivedAuthKey = parts[1];

    // Verify that the received auth key matches the expected one
    return receivedAuthKey == authKey;
}

std::string createMessage(const std::string& message, const std::string& authKey) {
    std::string combinedMessage = message + "|" + authKey;
    int messageSize = combinedMessage.size() + 1;
    return std::to_string(messageSize) + "|" + combinedMessage;
}

std::string messages::processMessage(const std::string& message, const std::string& authKey) const
{
    // Verify the auth key
    if (!verifyAuthKey(message, authKey)) {
        return "ERROR: Invalid authentication key.";
    }

    // Extract the message contents
    std::vector<std::string> parts = splitString(message, '|');

    // Verify that the message has at least two parts
    if (parts.size() < 2) {
        return "ERROR: Invalid message format.";
    }

    // Extract the message text
    std::string messageText = parts[0];

    // Capitalize the message text
    std::transform(messageText.begin(), messageText.end(), messageText.begin(), toupper);

    // Create and return the response message
    return createMessage(messageText, authKey);
}

int messages::serializedLen(unsigned int contentLen, unsigned int tagLen)
{
    return sizeof(messages::Header) + contentLen + tagLen;
}


char* messages::serialize(messages::MessageType type, const char* content, unsigned int contentLen, const unsigned char* authTag, unsigned int tagLen)
{
    // Calculate the total message length
    const unsigned int messageLen = sizeof(messages::Header) + contentLen + tagLen;

    // Allocate memory for the serialized message
    char* serializedMessage = new char[messageLen];

    // Initialize the message header
    messages::Header* header = reinterpret_cast<messages::Header*>(serializedMessage);
    header->magicNumber = htonl(MAGIC_NUMBER);
    header->type = htons(static_cast<uint16_t>(type));
    header->length = htonl(contentLen);

    // Copy the message content into the serialized message
    char* messageContent = serializedMessage + sizeof(messages::Header);
    std::memcpy(messageContent, content, contentLen);

    // Copy the authentication tag into the serialized message
    if (authTag != nullptr && tagLen > 0) {
        char* tag = messageContent + contentLen;
        std::memcpy(tag, authTag, tagLen);
    }

    return serializedMessage;
}


