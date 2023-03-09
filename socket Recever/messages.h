#pragma once
#include <xstring>

class messages
{
public:
    enum class MessageType : uint16_t {
        None = 0,
        Request = 1,
        Response = 2,
        Error = 3,
        Custom = 4,
        AuthResponse = 5
    };

    struct Header {
        uint32_t magicNumber;
        uint16_t type;
        uint32_t length;
    };

    static const uint32_t MAGIC_NUMBER = 0xDEADBEEF;
    
    std::string processMessage(const std::string& message, const std::string& authKey) const;
    static int serializedLen(unsigned messageLen, unsigned authTagLen);
    static char* serialize(MessageType type, const char* content, unsigned contentLen, const unsigned char* authTag,
                           unsigned tagLen);

private:

};

