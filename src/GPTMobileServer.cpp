#include <stdlib.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include "Core.h"


//For Sockets in windows
//#include <WinSock2.h> // for sockets
//#include <Ws2tcpip.h> // for internet addresses

//For Sockets in linux. 
#include <sys/socket.h> // for sockets
#include <arpa/inet.h> // for internet addresses
#include <unistd.h> // for close() function



//TODO: Get TCP Socket server running

int main(int argc, char **argv) {    
    bool exit_flag = false;
    Core* DeskGPTCore = new Core();
    
    std::string user_in = "";
    std::cout <<"Question: ";
    while (!exit_flag  && std::getline(std::cin, user_in)) {
        if (user_in == "--exit") {
            std::cout << "shutting down" << std::endl;
            exit_flag = true;
            continue;
        } else if (user_in == "--clear" || user_in == "-c") {
            system("clear");
            std::cout <<"Question: ";
            continue;
        } else if (user_in == "--help" || user_in == "-h") {
            Core::DisplayHelp();
            std::cout <<"Question: ";
            continue;
        } else {
            DeskGPTCore->SetCommand(user_in);
        }      
        DeskGPTCore->ExecuteCommand();
        user_in = "";
        std::cout << std::endl;
        std::cout <<"Question: ";
    }
    
}

/*

//TODO: This is linux boilerplate that needs to be implemented for server

int server_fd = socket(AF_INET, SOCK_STREAM, 0);
int opt = 1;
setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

struct sockaddr_in address;
int addrlen = sizeof(address);
address.sin_family = AF_INET;
address.sin_addr.s_addr = INADDR_ANY;
address.sin_port = htons(PORT);

bind(server_fd, (struct sockaddr *)&address, sizeof(address));
listen(server_fd, 3);
int new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
char buffer[1024] = {0};
int valread = recv(new_socket, buffer, 1024, 0);
send(new_socket, "Hello, client", strlen("Hello, client"), 0);
close(server_fd);

*/