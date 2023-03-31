#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include "BPHttp.h"
#include <netdb.h>

int main(int argc, char const *argv[]) {
    // Create a socket
    std::string user_input = "";    
    bool skip = false;
    char buffer[(1024*1024*5)] = {0};
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        std::cerr << "Error creating socket" << std::endl;
        return -1;
    }
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(1337);
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");    
    if (connect(clientSocket, (struct sockaddr*) &serverAddress, sizeof(serverAddress)) < 0) {
        std::cerr << "Error connecting to server" << std::endl;
        return -1;
    }
    
    std::cout <<"Query: ";    
    while (std::getline(std::cin, user_input)) {
        if (user_input == "--exit") {
            break;
        } else if (user_input == "--clear" || user_input == "") {
            system("clear");
            skip = true;
        } else {
            send(clientSocket, user_input.c_str(), strlen(user_input.c_str()), 0);
        }
        if (!skip) {
            // Receive a message from the server
            ssize_t valread = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (valread == -1) {            
                std::cout << "Error reading socket: Shutting down socket client." << std::endl;
                break;
            } else if (valread == 0) {
                std::cout << "Socket shut down from server: disconnecting." << std::endl;
                break;
            } else {
                std::cout << std::string(buffer) << std::endl;
            }
            std::cout << std::endl;
        }
        user_input = "";
        skip = false;
        memset(buffer, 0, sizeof(buffer));
        std::cout <<"Query: ";
    }    
    close(clientSocket);
    return 0;
}
