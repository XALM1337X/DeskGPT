#include "CoreServer.h"
#include "log.h"
#include "BPStrings.h"
#include "BPFile.h"
#include "BPExec.h"
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
    this->debug_mode = false;
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

void CoreServer::ToggleDebugMode() {
    if (this->debug_mode) {
        this->debug_mode = false;
    } else {
        this->debug_mode = true;
    }
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
                            this->StopAcceptHandler();
                        } else if (result == "debug-mode-toggle") {
                            std::cout << std::to_string(this->debug_mode) << std::endl;
                            send(new_socket, std::string("Debug mode toggled.").c_str(), strlen(std::string("Debug mode toggled.").c_str()), 0);
                        } else {
                            int bytesSent = send(new_socket, result.c_str(), strlen(result.c_str()), 0);
                            if (bytesSent < 0) {
                                ErrorLog::WriteLog("CoreServer::AcceptHandler[Lambda:handler_thread]:error - Failed to send message to client socket, shuttdown down connection...");
                                break;
                            } else {
                                if (this->debug_mode) {
                                    std::cout << "Response Sent:" <<std::endl;
                                }                                
                            }   
                        }
                    } else {
                        ErrorLog::WriteLog("CoreServer::AcceptHandler:error - HandleMessage failed.");
                        if (this->debug_mode) {
                            std::cout << buffer << std::endl;
                        }                        
                        std::string err_str = "Internal server error";
                        send(new_socket, err_str.c_str(), strlen(err_str.c_str()), 0);
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
    std::string pattern = "(POST|GET|PUT|HEAD|DELETE|CONNECT|OPTIONS|TRACE)\\s+([a-zA-Z\\/.-]+)\\s+([a-zA-Z]+)\\/([0-9.]+)\r*$";
    std::regex regex(pattern);
    std::smatch match;
    std::vector splt_str = BPStrings::SplitString(msg,'\n');
    //std::cout << msg << std::endl;
    if (msg == "--shutdown-server") {
        MainLog::WriteLog("CoreServer::HandleMessage:info - Shutting Down");
        if (this->debug_mode) {
            std::cout << "shutting down" << std::endl;
        }        
        ret = "server-shutdown-procedure";
    } else if(msg == "--debug-toggle") {
        ret = "debug-mode-toggle";
        this->ToggleDebugMode();
    } else if(msg == "--help") {
        return this->GetHelp();
    } else if (std::regex_match(splt_str[0], match, regex)) {
        //TODO:// Handle HTTP request
        return splt_str[0].c_str();        
    } else {   
        std::string esc_msg = BPStrings::EscapeStringCharacters(msg);
        this->SetCommand(esc_msg);
        ret = this->ExecuteGPTCommand();
    } 
    return ret;
}

void CoreServer::SetCommand(std::string command_str) {
    std::string command =  "curl -s https://api.openai.com/v1/chat/completions " 
                           "-H \"Authorization: Bearer "+this->api_key+"\" " 
                           "-H \"Content-Type: application/json\" " 
                           "-d '{ \"model\": \"gpt-3.5-turbo\", \"messages\": [{\"role\": \"user\", \"content\": \""+command_str+"\"}]}' ";
    

    this->current_command = command;
}

std::string CoreServer::ExecuteGPTCommand() {    
    std::string ret ="";      
    std::smatch match;
    std::string pattern = "^.*\n+(.*)$";
    std::regex regex(pattern);
    BPExecResult ex = BPExec::Exec(this->GetCommandString());
    if (ex.exit_code != 0) {
        ErrorLog::WriteLog("Core::ExecuteCommand:error - Failed to execute command.");
        return ret;
    }    
    rapidjson::Document document;
    rapidjson::ParseResult result = document.Parse(ex.result.c_str());    
    if (!result || document.HasMember("error")) {
        ErrorLog::WriteLog("Core::ExecuteCommand:error - Failed parsing JSON");
        return "Failed parsing JSON";
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
    } 
    return ret;
}

std::string CoreServer::GetCommandString() {
    return this->current_command;
}

std::string CoreServer::GetHelp() {
    std::string help_str = (
            "DeskGPT:\n"
            "  OPTIONS:\n"
            "    --help or -h       To see this option screen\n"
            "    --exit             To shutdown program\n"
            "    --debug-toggle     Toggle server debug mode\n"
            "    --shutdown-server  Shutdown server\n"
            "    --test-http        Send Test HTTP message to server\n"
            );
    return help_str;
}

bool CoreServer::ReadAPIKey() {
    std::string key = "";
    bool success = BPFile::FileReadString("/root/GPTMobileServer/src/etc/api.key", &key);
    if (!success || key == "") {
        if (key == "") {    
            ErrorLog::WriteLog("Core::ReadAPIKey:error - api.key file is empty.");
        } else {
            ErrorLog::WriteLog("Core::ReadAPIKey:error - Failed to read api.key file");
        }        
        return false;
    }
    this->api_key = key;    
    return true;
}