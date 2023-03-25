#include <stdlib.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include "Core.h"
#include "CoreServer.h"
//For Sockets in windows
//#include <WinSock2.h> // for sockets
//#include <Ws2tcpip.h> // for internet addresses

int main(int argc, char **argv) {    
    Core* DeskGPTCore = new Core();    
    while (DeskGPTCore->CoreServerObj->isRunning){}
    delete(DeskGPTCore);    
}
