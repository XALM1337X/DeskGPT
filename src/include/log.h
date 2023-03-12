#ifndef _LOG_H_
#define _LOG_H_
#include <string>
class Logger {
    public:
    virtual void WriteLog(std::string);

};


class MainLog {
    public:
        static void WriteLog(std::string);
};


class ErrorLog {
    public:
        static void WriteLog(std::string);
};


#endif