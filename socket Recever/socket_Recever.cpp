#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h> // Include the Winsock library

#pragma comment(lib, "ws2_32.lib") // Link with the Winsock library

#define BUFFER_SIZE 1024
#define PORT 12345
#define AUTH_KEY "my_secret_key"

int main()
{
    WSADATA wsa;
    int result = WSAStartup(MAKEWORD(2, 2), &wsa);
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
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1"); // Change to the server's IP address
    serverAddress.sin_port = htons(PORT);

    result = connect(connectSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    if (result == SOCKET_ERROR) {
        printf("connect failed: %d\n", WSAGetLastError());
        closesocket(connectSocket);
        WSACleanup();
        return 1;
    }

    printf("Connected to server on port %d.\n", PORT);

    char authBuffer[BUFFER_SIZE];
    snprintf(authBuffer, BUFFER_SIZE, "%s", AUTH_KEY);
    int authSize = strlen(authBuffer) + 1;

    result = send(connectSocket, (char*)&authSize, sizeof(int), 0);
    if (result == SOCKET_ERROR) {
        printf("send failed: %d\n", WSAGetLastError());
        closesocket(connectSocket);
        WSACleanup();
        return 1;
    }

    result = send(connectSocket, authBuffer, authSize, 0);
    if (result == SOCKET_ERROR) {
        printf("send failed: %d\n", WSAGetLastError());
        closesocket(connectSocket);
        WSACleanup();
        return 1;
    }

    char buffer[BUFFER_SIZE];
    int bytesReceived;

    int packetSize;
    bytesReceived = recv(connectSocket, (char*)&packetSize, sizeof(int), 0);
    if (bytesReceived == SOCKET_ERROR) {
        printf("recv failed: %d\n", WSAGetLastError());
        closesocket(connectSocket);
        WSACleanup();
        return 1;
    }

    if (bytesReceived == 0) {
        printf("Server disconnected.\n");
        closesocket(connectSocket);
        WSACleanup();
        return 1;
    }

    char* packet = (char*)malloc(packetSize);
    memset(packet, 0, packetSize);

    bytesReceived = recv(connectSocket, packet, packetSize, 0);
    if (bytesReceived == SOCKET_ERROR) {
        printf("recv failed: %d\n", WSAGetLastError());
        closesocket(connectSocket);
        free(packet);
        WSACleanup();
        return 1;
    }

    char messageBuffer[BUFFER_SIZE];
    printf("Enter a message to send to the server: ");
    fgets(messageBuffer, BUFFER_SIZE, stdin);
    char combinedBuffer[BUFFER_SIZE];
    //snprintf(authBuffer, BUFFER_SIZE, "%s", AUTH_KEY);
    snprintf(authBuffer, BUFFER_SIZE, "%s%s", messageBuffer, AUTH_KEY);
    int combinedSize = strlen(combinedBuffer) + 1;

    result = send(connectSocket, (char*)&combinedSize, sizeof(int), 0);

    
    if (result == SOCKET_ERROR) {
        printf("send failed: %d\n", WSAGetLastError());
        closesocket(connectSocket);
        WSACleanup();
        return 1;
    }

    result = send(connectSocket, combinedBuffer, combinedSize, 0);
    if (result == SOCKET_ERROR) {
        printf("send failed: %d\n", WSAGetLastError());
        closesocket(connectSocket);
        WSACleanup();
        return 1;
    }

    // Receive the packet data
    bytesReceived = recv(connectSocket, packet, packetSize, 0);
    if (bytesReceived == SOCKET_ERROR) {
        printf("recv failed: %d\n", WSAGetLastError());
        closesocket(connectSocket);
        free(packet);
        WSACleanup();
        return 1;
    }

    // Print the response from the server
    printf("Server response: %s\n", packet);

    // Clean up
    free(packet);
    closesocket(connectSocket);
    WSACleanup();
    return 0;
}