#ifndef CORE_H
#define CORE_H
#include <string>
class CoreServer;
class MainLog;
class ErrorLog;
class Core {
    //Methods/Functions
    public:
        Core();
        ~Core();
        void Init();
        void CleanUp();
    private:

    //Members
    public:        
        CoreServer* CoreServerObj;
    private:
};
#endif