#include <stdlib.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include "Core.h"


int main(int argc, char **argv) {
    //TODO: implement session ID system for context preservation.
    //-H 'Session-Id: <your_session_id>'
    
    bool exit_flag = false;
    Core* DeskGPTCore = new Core();
    //TODO://Need to read this key from file instead of CLI
    DeskGPTCore->api_key = std::string(argv[1]);
    
    std::string user_in = "";
    std::cout <<"Question: ";
    while (!exit_flag  && std::getline(std::cin, user_in)) {
        if (user_in == "--exit") {
            std::cout << "shutting down" << std::endl;
            exit_flag = true;
            continue;
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
        }      
        DeskGPTCore->ExecuteCommand();
        user_in = "";
        std::cout << std::endl;
        std::cout <<"Question: ";
    }
    
}



