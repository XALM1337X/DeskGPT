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



void CoreServer::Init() {
    //std::cout << "Init-Test:" << std::endl;
}