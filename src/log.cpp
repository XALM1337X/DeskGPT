#include "log.h"

void MainLog::WriteLog(std::string line) {
    std::string log = "echo "+line+" >> /root/GPTMobileServer/src/logs/MainLog.log";
    system(log.c_str());
}

void ErrorLog::WriteLog(std::string line) {
    std::string log = "echo "+line+" >> /root/GPTMobileServer/src/logs/ErrorLog.log";
    system(log.c_str());
}