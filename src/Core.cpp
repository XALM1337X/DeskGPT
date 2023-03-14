#include "Core.h"
#include "log.h"
#include "CoreServer.h"

Core::Core() {
    MainLog::WriteLog("Core::~Core: Initializing...");
    this->CoreServerObj = new CoreServer();
    this->Init();
}

Core::~Core() {
    MainLog::WriteLog("Core::~Core: Shutting Down...");
    delete(this->CoreServerObj);
}
void Core::Init() {
    
}


void Core::CleanUp() {
    //Cleanup memory before shutdown.
    delete(this->CoreServerObj);
}