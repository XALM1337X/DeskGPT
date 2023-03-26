#include "Core.h"
#include "BPLog.h"
#include "CoreServer.h"

Core::Core() {
    BPMainLog::WriteLog("Core::~Core: Initializing...","/root/GPTMobileServer/src/logs/MainLog.log");
    this->CoreServerObj = new CoreServer();
    this->Init();
}

Core::~Core() {
    BPMainLog::WriteLog("Core::~Core: Shutting Down...","/root/GPTMobileServer/src/logs/MainLog.log");
    delete(this->CoreServerObj);
}
void Core::Init() {
    
}


void Core::CleanUp() {
    //Cleanup memory before shutdown.
    delete(this->CoreServerObj);
}