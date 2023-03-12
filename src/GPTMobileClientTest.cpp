#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

int main(int argc, char const *argv[]) {
    // Create a socket
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (clientSocket < 0) {
        std::cerr << "Error creating socket" << std::endl;
        return -1;
    }

    // Set up the address structure for the server
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Connect to the server
    if (connect(clientSocket, (struct sockaddr*) &serverAddress, sizeof(serverAddress)) < 0) {
        std::cerr << "Error connecting to server" << std::endl;
        return -1;
    }

    // Send a message to the server
    char message[] = "Hello, server!";
    send(clientSocket, message, strlen(message), 0);

    // Receive a message from the server
    char buffer[1024] = {0};
    ssize_t receivedBytes = recv(clientSocket, buffer, sizeof(buffer), 0);

    if (receivedBytes < 0) {
        std::cerr << "Error receiving message from server" << std::endl;
    } else {
        std::cout << "Received message from server: " << buffer << std::endl;
    }

    // Close the socket
    close(clientSocket);

    return 0;
}
