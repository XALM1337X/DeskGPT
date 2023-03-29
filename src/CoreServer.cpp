#include "CoreServer.h"
#include "BPLog.h"
#include "BPStrings.h"
#include "BPFile.h"
#include "BPExec.h"
#include "BPHttp.h"
#include "rapidjson/document.h"
#include "rapidjson/error/en.h"
#include <string>
#include <regex>
#include <fstream>
#include <iostream>
#include <cstring>
#include <thread>
#include <vector>

CoreServer::CoreServer() {
    std::cout << "CoreServer::CoreServer: Initializing..." << std::endl;
    BPLog::WriteLog("CoreServer::CoreServer: Initializing...","/root/GPTMobileServer/src/logs/MainLog.log");
    this->isRunning = false;
    this->debug_mode = false;
    this->current_command ="";
    this->api_key = "";
    this->web_root = "/root/GPTMobileServer/src/www/";    
    this->Init();
}

CoreServer::~CoreServer() {
    std::cout << "CoreServer::~CoreServer: Shutting Down..." << std::endl;
    BPLog::WriteLog("CoreServer::~CoreServer: Shutting Down...","/root/GPTMobileServer/src/logs/MainLog.log");
}

void CoreServer::Init() {
    if (!this->ReadAPIKey()){
        return;
    }    
    this->server_socket = socket(AF_INET, SOCK_STREAM, 0);
    this->opt = 1;
    setsockopt(this->server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &this->opt, sizeof(this->opt));    
    this->address.sin_family = AF_INET;
    this->address.sin_addr.s_addr = htonl(INADDR_ANY);
    this->address.sin_port = htons(1337);    
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

    struct sockaddr_in client_ip;
    socklen_t client_iplen = sizeof(client_ip);
    char ip_address[INET_ADDRSTRLEN];

    BPLog::WriteLog("Starting Accept Handler","/root/GPTMobileServer/src/logs/MainLog.log");
    int addrlen = sizeof(this->address);
    int new_socket;
    while (this->isRunning) {
        new_socket = accept(this->server_socket, (struct sockaddr *)&this->address, (socklen_t *)&addrlen);
        if (new_socket < 0) {
            BPErrorLog::WriteLog("CoreServer::AcceptHandler:error - Error accepting new connection","/root/GPTMobileServer/src/logs/ErrorLog.log");
            std::cout << "Error accepting new connection" << std::endl;;
            continue;
        } 

        if (getpeername(new_socket, (struct sockaddr *)&client_ip, &client_iplen) < 0) {
            BPErrorLog::WriteLog("CoreServer::AcceptHandler:error - Failed to get client IP.", "/root/GPTMobileServer/src/logs/ErrorLog.log");
        } else {
            inet_ntop(AF_INET, &(client_ip.sin_addr), ip_address, INET_ADDRSTRLEN);
            BPLog::WriteLog("CoreServer::AcceptHandler: IP Address: "+std::string(ip_address)+" connected!", "/root/GPTMobileServer/src/logs/MainLog.log");
            if (this->debug_mode) {
                std::cout << "IP Address: "<<ip_address<< " connected!" << std::endl;
            }            
        }




        std::thread handler_thread([this, new_socket]() {
           this->LaunchHandlerInternals(new_socket);
        });
        handler_thread.detach();
    }
}

void CoreServer::LaunchHandlerInternals(int socket) {
    //TODO: Replace all send() functions data to HTTP messages
    char buffer[(1024*1024*5)] = {0};
    while(this->isRunning) {
        int valread = recv(socket, buffer, (1024*1024*5), 0);
        if (valread == -1) {
            BPErrorLog::WriteLog("CoreServer::LaunchHandlerThread[Lambda:handler_thread]:error - Socket received error. Closing socket: "+std::to_string(socket),"/root/GPTMobileServer/src/logs/ErrorLog.log");
            break;
        } else if (valread == 0) {
            BPLog::WriteLog("CoreServer::LaunchHandlerThread[Lambda:handler_thread]: - Client has disconnected from socket, shutting down connection...","/root/GPTMobileServer/src/logs/MainLog.log");
            break;
        } else {
            std::string msg_str(buffer);
            std::string result = this->HandleMessage(msg_str, socket);
            if (result != "") {
                if (result == "server-shutdown-procedure") {
                    send(socket, std::string("").c_str(), strlen(std::string("").c_str()), 0);
                    this->StopAcceptHandler();
                } else if (result == "debug-mode-toggle") {
                    std::cout << std::to_string(this->debug_mode) << std::endl;
                    send(socket, std::string("Debug mode toggled.").c_str(), strlen(std::string("Debug mode toggled.").c_str()), 0);
                } else {
                    int bytesSent = send(socket, result.c_str(), strlen(result.c_str()), 0);
                    if (bytesSent < 0) {
                        BPErrorLog::WriteLog("CoreServer::LaunchHandlerThread[Lambda:handler_thread]:error - Failed to send message to client socket, shuttdown down connection...","/root/GPTMobileServer/src/logs/ErrorLog.log");
                        break;
                    } else {
                        if (this->debug_mode) {
                            std::cout << "Response Sent:" <<std::endl;
                        }                                
                    }   
                }
            } else {
                BPErrorLog::WriteLog("CoreServer::LaunchHandlerThread:error - HandleMessage failed.","/root/GPTMobileServer/src/logs/ErrorLog.log");
                if (this->debug_mode) {
                    std::cout << buffer << std::endl;
                }                        
                std::string err_str = "Internal server error";
                send(socket, err_str.c_str(), strlen(err_str.c_str()), 0);
            }
        }
        memset(buffer, 0, sizeof(buffer));
    }
    close(socket); 
}

std::string CoreServer::HandleMessage(std::string msg, int new_socket) {
    std::string ret = "";
    std::string pattern = "([A-Z]+)\\s+([a-zA-Z0-9\\-._~:\\/?#\\[\\]@!$&'()*+,;=]+)\\s+([a-zA-Z]+)\\/([0-9.]+)\r*$";
    std::string pattern_resp = "HTTP\\/([0-9.]+)\\s+([0-9]+)\\s+(.*)\r*";
    std::regex regex(pattern);
    std::regex regex2(pattern_resp);
    std::smatch match;
    std::vector<std::string> splt_str = BPStrings::SplitString(msg,'\n');   
    if (msg == "--shutdown-server") {
        BPLog::WriteLog("CoreServer::HandleMessage:info - Shutting Down","/root/GPTMobileServer/src/logs/MainLog.log");
        if (this->debug_mode) {
            std::cout << "shutting down" << std::endl;
        }        
        ret = "server-shutdown-procedure";
    } else if(msg == "--debug-toggle") {        
        this->ToggleDebugMode();
        ret = "debug-mode-toggle";
    } else if(msg == "--help") {
        ret = this->GetHelp();
    } else if (msg == "--server-clear") {
        system("clear");
        ret = "response";
    } else if (std::regex_match(splt_str[0], match, regex) || std::regex_match(splt_str[0], match, regex2)) {
        ret = this->HandleHTTPMessage(splt_str);        
    } else {   
        std::string esc_msg = BPStrings::EscapeStringCharacters(msg);
        this->SetCommand(esc_msg);
        ret = this->ExecuteGPTCommand();
    } 
    return ret;
}

std::string CoreServer::HandleHTTPMessage(std::vector<std::string> lines) {

    std::string mime_html_regex_pattern =".*\\.html.*";
    std::string mime_js_regex_pattern =".*\\.js.*";
    std::string mime_css_regex_pattern =".*\\.css.*";
    std::string mime_img_regex_pattern =".*(\\.png|\\.jpg\\|\\.svg|\\.ico).*";
    std::regex regex_html(mime_html_regex_pattern);
    std::regex regex_js(mime_js_regex_pattern);
    std::regex regex_css(mime_css_regex_pattern);
    std::regex regex_img(mime_img_regex_pattern);
    std::smatch match;

    BPHttpMessage response;
    BPHttpMessage msg;

    if (!msg.Parse(lines)) {
        //TODO:// Return failure to parse request
    }
    std::string resource = msg.ParseRequestResource(msg.entity_head);
    if (resource == "") {
        std::cout << "Failed to Parse Resource"<< std::endl;
        //TODO:// Return failure to parse entity header resource.
    }
    std::string* file_str = new std::string("");
    bool read_succes = false;    
    if (std::regex_search(resource, regex_html)) {
        if (BPFile::FileExists(this->web_root+"html"+resource)) {
            read_succes = BPFile::FileReadString(this->web_root+"html"+resource, file_str);
        }
    } else if (std::regex_search(resource, regex_js)) {
        if (BPFile::FileExists(this->web_root+"js"+resource)) {
            read_succes = BPFile::FileReadString(this->web_root+"js"+resource, file_str);
        }
    } else if (std::regex_search(resource, regex_css)) {
        if (BPFile::FileExists(this->web_root+"css"+resource)) {
            read_succes = BPFile::FileReadString(this->web_root+"css"+resource, file_str);
        }
    } else if (std::regex_search(resource, regex_img)) {
        if (BPFile::FileExists(this->web_root+"img"+resource)) {
            read_succes = BPFile::FileReadString(this->web_root+"img"+resource, file_str);
        }
    } 

    if (!read_succes) {
        std::string response = "HTTP/1.1 404 Not Found\r\n"
                       "Content-Type: text/plain\r\n"
                       "Content-Length: 13\r\n"
                       "\r\n"
                       "404 Not Found";
        return response;
    }    
    //Testing shit/////////////////////////
    std::string http_msg = "HTTP/1.1 200 OK\r\n"
                           "Content-Type: text/html\r\n"
                           "Content-Length: "+std::to_string((*file_str).size())+"\r\n"
                           "\r\n"+
                           (*file_str);
    ///////////////////////////////////////


    return http_msg;
}

void CoreServer::SetCommand(std::string command_str) {
    std::string command =  "curl -s https://api.openai.com/v1/chat/completions "
                           "-H \"Authorization: Bearer "+this->api_key+"\" "
                           "-H \"Content-Type: application/json\" "
                           "-d '{\"model\": \"gpt-3.5-turbo\", \"messages\": [{ \"role\": \"user\", \"content\": \""+command_str+"\"}]}' ";
    

    this->current_command = command;
}

std::string CoreServer::ExecuteGPTCommand() {    
    std::string ret ="";      
    std::smatch match;
    std::string pattern = "^.*\n+(.*)$";
    std::regex regex(pattern);
    BPExecResult ex = BPExec::Exec(this->GetCommandString(), true);
    if (ex.exit_code != 0) {
        BPErrorLog::WriteLog("Core::ExecuteCommand:error - Failed to execute command.","/root/GPTMobileServer/src/logs/ErrorLog.log");
        return ret;
    }
    std::cout << ex.result << std::endl;    
    rapidjson::Document document;
    rapidjson::ParseResult result = document.Parse(ex.result.c_str());    
    if (!result || document.HasMember("error")) {
        std::cout << rapidjson::GetParseError_En(result.Code()) << std::endl;
        BPErrorLog::WriteLog("Core::ExecuteCommand:error - Failed parsing JSON: "+rapidjson::GetParseError_En(result.Code()),"/root/GPTMobileServer/src/logs/ErrorLog.log");
        return "Failed parsing JSON: "+rapidjson::GetParseError_En(result.Code());
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
            BPErrorLog::WriteLog("Core::ReadAPIKey:error - api.key file is empty.","/root/GPTMobileServer/src/logs/ErrorLog.log");
        } else {
            BPErrorLog::WriteLog("Core::ReadAPIKey:error - Failed to read api.key file","/root/GPTMobileServer/src/logs/ErrorLog.log");
        }        
        return false;
    }
    this->api_key = key;    
    return true;
}