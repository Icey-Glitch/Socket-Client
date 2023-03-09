#pragma once

class Packet
{
public:
    static int receiveAll(int socket, char* buffer, int length);
    static int receivePacketSize(int socket);
};
