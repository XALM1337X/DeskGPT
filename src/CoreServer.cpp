#include "CoreServer.h"
#include "log.h"
#include "BPTools/BPStrings.h"
#include "BPTools/BPFile.h"
#include "BPTools/BPExec.h"
#include "rapidjson/document.h"
#include <string>
#include <regex>
#include <fstream>
#include <iostream>
#include <cstring>
#include <thread>
#include <vector>

CoreServer::CoreServer() {
    std::cout << "CoreServer::CoreServer: Initializing..." << std::endl;
    MainLog::WriteLog("CoreServer::CoreServer: Initializing...");
    this->isRunning = false;
    this->current_command ="";
    this->api_key = "";
    this->Init();
}

CoreServer::~CoreServer() {
    std::cout << "CoreServer::~CoreServer: Shutting Down..." << std::endl;
    MainLog::WriteLog("CoreServer::~CoreServer: Shutting Down...");
}

void CoreServer::Init() {
    if (!this->ReadAPIKey()){
        return;
    }    
    this->server_socket = socket(AF_INET, SOCK_STREAM, 0);
    this->opt = 1;
    setsockopt(this->server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &this->opt, sizeof(this->opt));    
    this->address.sin_family = AF_INET;
    this->address.sin_addr.s_addr = INADDR_ANY;
    this->address.sin_port = htons(8080);    
    bind(this->server_socket, (struct sockaddr *)&this->address, sizeof(this->address));
    listen(this->server_socket, 3);
    this->isRunning = true;
    this->StartAcceptHandler();    
}

void CoreServer::StartAcceptHandler() {
    std::thread accept_thread([this]() {
        this->AcceptHandler();
    });
    accept_thread.detach();
}

void CoreServer::StopAcceptHandler() {
    this->isRunning = false;
}

void CoreServer::AcceptHandler() {    
    MainLog::WriteLog("Starting Accept Handler");
    int addrlen = sizeof(this->address);
    int new_socket;
    while (this->isRunning) {
        new_socket = accept(this->server_socket, (struct sockaddr *)&this->address, (socklen_t *)&addrlen);
        if (new_socket < 0) {
            MainLog::WriteLog("CoreServer::AcceptHandler:error - Error accepting new connection");
            std::cout << "Error accepting new connection" << std::endl;;
            continue;
        }

        std::thread handler_thread([this, new_socket]() {
            char buffer[(1024*1024*5)] = {0};
            while(this->isRunning) {
                int valread = recv(new_socket, buffer, (1024*1024*5), 0);
                if (valread == -1) {
                    ErrorLog::WriteLog("CoreServer::AcceptHandler[Lambda:handler_thread]:error - Socket received error. Closing socket: "+std::to_string(new_socket));
                    break;
                } else if (valread == 0) {
                    MainLog::WriteLog("CoreServer::AcceptHandler[Lambda:handler_thread]: - Client has disconnected from socket, shutting down connection...");
                    break;
                } else {
                    std::string msg_str(buffer);
                    std::string result = this->HandleMessage(msg_str, new_socket);
                    if (result != "") {
                        if (result == "server-shutdown-procedure") {
                            send(new_socket, std::string("").c_str(), strlen(std::string("").c_str()), 0);
                        } else {
                        int bytesSent = send(new_socket, result.c_str(), strlen(result.c_str()), 0);
                        if (bytesSent < 0) {
                            ErrorLog::WriteLog("CoreServer::AcceptHandler[Lambda:handler_thread]:error - Failed to send message to client socket, shuttdown down connection...");
                            break;
                        } else {
                            std::cout << "Sending GPT response:" <<std::endl;
                        }                       
                    } else {
                        ErrorLog::WriteLog("CoreServer::AcceptHandler:error - HandleMessage failed.");
                    }
                }
                memset(buffer, 0, sizeof(buffer));
            }
            close(new_socket);            
        });
        handler_thread.detach();
    }
}

std::string CoreServer::HandleMessage(std::string msg, int new_socket) {
    std::string ret = "";
    if (msg == "--shutdown-server") {
        std::cout << "shutting down" << std::endl;
        MainLog::WriteLog("CoreServer::HandleMessage:info - Shutting Down");
        this->StopAcceptHandler();
    } else if(msg == "--help") {
        std::string help_msg = this->GetHelp();
        send(new_socket, help_msg.c_str(), strlen(help_msg.c_str()), 0);
    } else {
        this->SetCommand(msg);
        std::cout <<"Recieved Query: "+msg << std::endl;
        ret = this->ExecuteCommand();
    } 
    return ret;
}

void CoreServer::SetCommand(std::string command_str) {
    std::string command = "";
    if (this->context_id != "") {
        command =  "curl -s https://api.openai.com/v1/chat/completions \
        -H \"Session-Id: "+this->context_id+"\" \
        -H \"Authorization: Bearer "+this->api_key+"\" \
        -H \"Content-Type: application/json\" \
        -d '{ \"model\": \"gpt-3.5-turbo\", \"messages\": [{\"role\": \"user\", \"content\": \""+command_str+"\"}]}'";
    } else {
        command =  "curl -s https://api.openai.com/v1/chat/completions \
        -H \"Authorization: Bearer "+this->api_key+"\" \
        -H \"Content-Type: application/json\" \
        -d '{ \"model\": \"gpt-3.5-turbo\", \"messages\": [{\"role\": \"user\", \"content\": \""+command_str+"\"}]}'";
    }

    this->current_command = command;
}

std::string CoreServer::ExecuteCommand() {    
    std::string ret ="";       
    char buffer[128];
    std::string result_str = "";
    std::smatch match;
    std::string pattern = "^.*\n+(.*)$";
    std::regex regex(pattern);
    FILE* pipe = popen(this->GetCommandString().c_str(),"r");  
    if (!pipe) {
        ErrorLog::WriteLog("Core::ExecuteCommand:error - Failed to open pipe");
    }
    while (!feof(pipe)) {
        if (fgets(buffer, 128, pipe) != NULL){
            result_str += buffer;
        }            
    }
    pclose(pipe);    
    rapidjson::Document document;
    rapidjson::ParseResult result = document.Parse(result_str.c_str());
    if (!result) {
        std::cout << "CoreServer::ExecuteCommand:error - Failed to parse JSON";
        ErrorLog::WriteLog("Core::ExecuteCommand:error - Failed parsing JSON");
    } else {
        const rapidjson::Value& choices = document["choices"];
        if (choices.IsArray()) {
            const rapidjson::Value& choice = choices[0];
            if (choice.IsObject()) {
                const rapidjson::Value& message = choice["message"];
                if (message.IsObject()) {
                    const rapidjson::Value& content = message["content"];
                    if (content.IsString()) {
                        std::cout << std::endl;
                        std::string str = std::string(content.GetString());
                        if (std::regex_match(str, match, regex)) {
                            ret = match[1];
                        } else {
                            ret = str;
                        }
                    }
                }
            }
        }
        const rapidjson::Value& id = document["id"];
        if (id.IsString()) {
            this->context_id = id.GetString();
        }
    }
    return ret;
}

std::string CoreServer::GetCommandString() {
    return this->current_command;
}

std::string CoreServer::GetHelp() {
    std::string help_str = ("NAME:\n"  
            "  DeskGPT\n"
            "USAGE:\n" 
            "  DeskGPT.exe [options] <api_key>\n"
            "OPTIONS:\n"
            "  --help or -h     To see this option screen\n"
            "  --exit           To shutdown program\n");
    return help_str;
}

bool CoreServer::ReadAPIKey() {
    std::ifstream file("/root/GPTMobileServer/src/etc/api.key");
    if (!file.is_open()) {
        ErrorLog::WriteLog("Core::ReadAPIKey:error - Failed opening api.key file");
        return false;
    } else {
        std::string line ="";
        std::getline(file,line);
        if (line == "") {
            ErrorLog::WriteLog("CoreServer::ReadAPIKey:error - Api key file is empty.");
            return false;
        } else {
            this->api_key = line;
        }
    }
    return true;
}