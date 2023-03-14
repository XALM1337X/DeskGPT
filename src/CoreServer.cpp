#include "CoreServer.h"
#include <iostream>
#include <cstring>
#include <thread>
#include "log.h"

CoreServer::CoreServer() {
    MainLog::WriteLog("CoreServer::~CoreServer: Initializing...");
    this->isRunning = new bool(false);
    this->Init();
}

CoreServer::~CoreServer() {
    MainLog::WriteLog("CoreServer::~CoreServer: Shutting Down...");
    delete(this->isRunning);
}



void CoreServer::Init() {
    //TODO: This is linux boilerplate that needs to be implemented for server
    this->server_fd = socket(AF_INET, SOCK_STREAM, 0);
    this->opt = 1;
    setsockopt(this->server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &this->opt, sizeof(this->opt));    
    this->address.sin_family = AF_INET;
    this->address.sin_addr.s_addr = INADDR_ANY;
    this->address.sin_port = htons(8080);    
    bind(this->server_fd, (struct sockaddr *)&this->address, sizeof(this->address));
    listen(this->server_fd, 3);
    *this->isRunning = true;
    this->StartAcceptHandler();    
}



void CoreServer::AcceptHandler() {    
    MainLog::WriteLog("Starting Accept Handler");
    int addrlen = sizeof(this->address);
    int new_socket;
    while (true && *this->isRunning) {
        new_socket = accept(this->server_fd, (struct sockaddr *)&this->address, (socklen_t *)&addrlen);
        if (new_socket < 0) {
            MainLog::WriteLog("CoreServer::AcceptHandler:error - Error accepting new connection");
            std::cout << "Error accepting new connection" << std::endl;;
            continue;
        }
        std::thread handler_thread([new_socket]() {
            char buffer[1024] = {0};
            int valread = recv(new_socket, buffer, 1024, 0);
            std::cout << buffer << std::endl;
            send(new_socket, "Hello, client", strlen("Hello, client"), 0);
            close(new_socket);
        });
        handler_thread.detach();
    }
}


bool CoreServer::ReadAPIKey() {
    std::ifstream file("/root/GPTMobileServer/src/etc/api.key");
    if (!file.is_open()) {
        ErrorLog::WriteLog("Core::ReadAPIKey:error - Failed opening api.key file");
        return false;
    } else {
        std::string line ="";
        std::getline(file,line);
        if (line == "") {
            ErrorLog::WriteLog("CoreServer::ReadAPIKey:error - Api key file is empty.");
            return false;
        } else {
            this->api_key = line;
        }
    }
    return true;
}