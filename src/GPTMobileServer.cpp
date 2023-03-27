#include "Core.h"
#include "CoreServer.h"
#include <stdlib.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include <chrono>
#include <thread>
int main(int argc, char **argv) {    
    Core* DeskGPTCore = new Core();        
    while (DeskGPTCore->CoreServerObj->isRunning){
        std::this_thread::sleep_for(std::chrono::milliseconds(10000));
    }
    delete(DeskGPTCore);    
}