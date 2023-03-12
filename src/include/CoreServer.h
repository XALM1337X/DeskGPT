#ifndef _CORE_SERVER_H_
#define _CORE_SERVER_H_
//#include <string>
#include <sys/socket.h> // for sockets
#include <arpa/inet.h> // for internet addresses
#include <unistd.h> // for close() function
class CoreServer {
    public:
        CoreServer();
        ~CoreServer();
        void Init();
        void StartAcceptHandler();
        void StopAcceptHandler();
        void AcceptHandler();
    private:

    public:
        int server_fd;
        int opt;
        struct sockaddr_in address;
        bool* isRunning;
    private:

};

#endif