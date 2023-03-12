#include <stdlib.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include "Core.h"
#include "CoreServer.h"


//For Sockets in windows
//#include <WinSock2.h> // for sockets
//#include <Ws2tcpip.h> // for internet addresses

//For Sockets in linux. 
//TODO: Get TCP Socket server running

int main(int argc, char **argv) {    
    Core* DeskGPTCore = new Core();    
    std::string user_in = "";
    std::cout <<"Question: ";
    while (*DeskGPTCore->CoreServerObj->isRunning == true) {
        while ( std::getline(std::cin, user_in)) {        
            if (user_in == "--exit") {
                std::cout << "shutting down" << std::endl;
                *DeskGPTCore->CoreServerObj->isRunning = false;
                break;
            } else if (user_in == "--clear" || user_in == "-c") {
                system("clear");
                std::cout <<"Question: ";
                continue;
            } else if (user_in == "--help" || user_in == "-h") {
                Core::DisplayHelp();
                std::cout <<"Question: ";
                continue;
            } else {
                DeskGPTCore->SetCommand(user_in);
                DeskGPTCore->ExecuteCommand();
            }        
            user_in = "";
            std::cout <<"Question: ";
            
        }
    }

    delete(DeskGPTCore);    
}
