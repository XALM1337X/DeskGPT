#ifndef CORE_SERVER_H
#define CORE_SERVER_H
//#include <string>
#include <sys/socket.h> // for sockets
#include <arpa/inet.h> // for internet addresses
#include <unistd.h> // for close() function
#include <string>
#include <atomic>
#include <vector>
class CoreServer {
    public:
        CoreServer();
        ~CoreServer();
        void Init();
        void StartAcceptHandler();
        void StopAcceptHandler();
        void ToggleDebugMode();
        void AcceptHandler();
        void SetCommand(std::string);
        void LaunchHandlerInternals(int);
        bool ReadAPIKey();
        std::string HandleMessage(std::string,int);    
        std::string HandleHTTPMessage(std::vector<std::string>);
        std::string ExecuteGPTCommand();
        std::string GetCommandString();
        std::string GetHelp();
    private:

    public:
        int server_socket;
        int opt;
        struct sockaddr_in address;
        std::atomic<bool> isRunning;
        std::atomic<bool> debug_mode;
        std::string current_command;
        std::string api_key;        
    private:

};

#endif