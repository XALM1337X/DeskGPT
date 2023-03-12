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


}