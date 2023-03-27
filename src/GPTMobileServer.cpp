#include "Core.h"
#include "CoreServer.h"
#include <stdlib.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include <chrono>
#include <thread>

//For Sockets in windows
//#include <WinSock2.h> // for sockets
//#include <Ws2tcpip.h> // for internet addresses

int main(int argc, char **argv) {    
    Core* DeskGPTCore = new Core();    
        
    while (DeskGPTCore->CoreServerObj->isRunning){
        std::this_thread::sleep_for(std::chrono::milliseconds(10000));
    }
    delete(DeskGPTCore);    
}
