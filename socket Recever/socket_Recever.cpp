#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include "messages.h"
#include "Ws2tcpip.h"
#include "auth.h"
#include "Packet.h"

#pragma comment(lib, "ws2_32.lib")

#define BUFFER_SIZE 1024
#define PORT 12345
#define SERVER_IP "127.0.0.1"



int main()
{
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        printf("WSAStartup failed: %d\n", result);
        return 1;
    }

    SOCKET connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (connectSocket == INVALID_SOCKET) {
        printf("socket failed: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    if (inet_pton(AF_INET, SERVER_IP, &serverAddress.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }
    serverAddress.sin_port = htons(PORT);

    result = connect(connectSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    if (result == SOCKET_ERROR) {
        printf("connect failed: %d\n", WSAGetLastError());
        closesocket(connectSocket);
        WSACleanup();
        return 1;
    }

    printf("Connected to server on port %d.\n", PORT);

    // Authenticate with the server
    if (auth::authWithServer(connectSocket, "mySecretKey") != 0) {
        printf("Authentication failed.\n");
        closesocket(connectSocket);
        WSACleanup();
        return 1;
    }

    char messageBuffer[BUFFER_SIZE];
    printf("Enter a message to send to the server: ");
    fgets(messageBuffer, BUFFER_SIZE, stdin);

    // Send the message to the server
    if (auth::sendToServer(connectSocket, messageBuffer) == SOCKET_ERROR) {
        printf("Failed to send message to server.\n");
        closesocket(connectSocket);
        WSACleanup();
        return 1;
    }

    // Receive the response from the server
    char* response = nullptr;
    int responseLen = 0;
    bool success = auth::receiveFromServer(connectSocket);

    if (!success) {
        printf("Failed to receive response from server.\n");
        closesocket(connectSocket);
        WSACleanup();
        return 1;
    }

    responseLen = Packet::receivePacketSize(connectSocket);
    response = new char[responseLen];
    if (!Packet::receiveAll(connectSocket, response, responseLen)) {
        printf("Failed to receive response from server.\n");
        delete[] response;
        closesocket(connectSocket);
        WSACleanup();
        return 1;
    }

    // Print the response from the server
    printf("Server response: %s\n", response);

    // Clean up
    free(response);
    closesocket(connectSocket);
    WSACleanup();

    return 0;
}
