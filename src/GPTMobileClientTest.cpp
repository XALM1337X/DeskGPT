#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

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
    std::cout <<"Query: ";    
    while (std::getline(std::cin, user_input)) {
        if (user_input == "--exit") {
            break;
        } else if (user_input == "--clear") {
            system("clear");
            skip = true;
        } else if (user_input == "--test_http") {
        }
        send(clientSocket, user_input.c_str(), strlen(user_input.c_str()), 0);
        // Receive a message from the server
        char buffer[(1024*1024*5)] = {0};
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
        user_input = "";
        memset(buffer, 0, sizeof(buffer));
        std::cout << std::endl;
        std::cout <<"Query: ";
    }    

    // Close the socket
    close(clientSocket);

    return 0;
}
