#include "Packet.h"

#include <iostream>

#include "auth.h"
#include "messages.h"

int Packet::receiveAll(int socket, char* buffer, int length) {
    int received = 0;
    while (received < length) {
        int result = recv(socket, buffer + received, length - received, 0);
        if (result == SOCKET_ERROR) {
            printf("recv failed: %d\n", WSAGetLastError());
            return result;
        }
        if (result == 0) {
            printf("Connection closed by peer.\n");
            return result;
        }
        received += result;
    }
    return received;
}

int Packet::receivePacketSize(int socket) {
    int packetSize;
    if (!Packet::receiveAll(socket, reinterpret_cast<char*>(&packetSize), sizeof(packetSize))) {
        printf("Failed to receive packet size.\n");
        return -1;
    }
    return packetSize;
}


