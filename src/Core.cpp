#include "Core.h"
#include <string>
#include <iostream>
#include "rapidjson/document.h"
#include <regex>
#include "CoreServer.h"
#include "log.h"
#include <fstream>

Core::Core() {
    this->current_command = "";
    this->api_key = "";
    this->context_id = "";
    this->CoreServerObj = new (CoreServer);
    this->Init();
}
void Core::Init() {
    this->ReadAPIKey();
}


void Core::ReadAPIKey() {
    std::ifstream file("/root/GPTMobileServer/src/etc/api.key");
    if (!file.is_open()) { // use !file.fail() for older compilers
        ErrorLog::WriteLog("Core::Init:error - Failed opening file");
    } else {
        std::string line ="";
        std::getline(file,line);
        this->api_key = line;
    }
}

void Core::DisplayHelp() {
 std::string help_str = ("NAME:\n"  
            "  DeskGPT\n"
            "USAGE:\n" 
            "  DeskGPT.exe [options] <api_key>\n"
            "OPTIONS:\n"
            "  --help or -h     To see this option screen\n"
            "  --clear or -c    To Clear the screen\n"
            "  --exit           To shutdown program\n");
    std::cout << help_str <<std::endl;
}

void Core::SetCommand(std::string command_str) {
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

std::string Core::GetCommandString() {
    return this->current_command;
}

void Core::ExecuteCommand() {       
    char buffer[128];
    std::string result_str = "";
    std::smatch match;
    std::string pattern = "^.*\n+(.*)$";
    std::regex regex(pattern);
    FILE* pipe = popen(this->GetCommandString().c_str(),"r");  
    if (!pipe) {
        std::cout << "Failed to open pipe" << std::endl;
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
        std::cout << "Error parsing JSON: " << result.Code() << std::endl;
        std::cout << result_str << std::endl;
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
                            std::cout << match[1] << std::endl;
                        } else {
                            std::cout << str << std::endl;
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
}