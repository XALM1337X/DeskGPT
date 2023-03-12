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
    //std::cout << "Init-Test:" << std::endl;
}