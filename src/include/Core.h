#ifndef _CORE_H_
#define _CORE_H_
#include <string>
class CoreServer;
class MainLog;
class ErrorLog;
class Core {
    //Methods/Functions
    public:
        Core();
        ~Core();
        static void DisplayHelp();
        void SetCommand(std::string);
        std::string GetCommandString();
        void ReadAPIKey();
        void ExecuteCommand();
        void Init();
        void CleanUp();
    private:

    //Members
    public:

        
        CoreServer* CoreServerObj;
        std::string current_command;
        std::string api_key;
        std::string context_id;
    private:
};
#endif