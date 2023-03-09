#ifndef _CORE_H
#define _CORE_H
#include <string>
class Core {
    //Methods/Functions
    public:
        Core();
        static void DisplayHelp();
        void SetCommand(std::string);
        std::string GetCommandString();
        void ExecuteCommand();
    private:

    //Members
    public:
        std::string current_command;
        std::string api_key;
        std::string context_id;
    private:
};
#endif