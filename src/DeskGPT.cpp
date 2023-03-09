#include <stdlib.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include <regex>
#include "rapidjson/document.h"

int main(int argc, char **argv) {
    //TODO: implement session ID system for context preservation.
    //-H 'Session-Id: <your_session_id>'
    std::string pattern = "^.*\n+(.*)$";
    std::regex regex(pattern);
    std::smatch match;
    char buffer[128];
    std::string result_str = "";
    bool exit_flag = false;

    //TODO://Need to read this key from file instead of CLI
    std::string key = std::string(argv[1]);
    std::string command = "";
    std::string user_in = "";
    std::cout <<"Question: ";
    while (!exit_flag && std::getline(std::cin, user_in)) {
        if (user_in == "--exit") {
            std::cout << "shutting down" << std::endl;
            exit_flag = true;
            continue;
        } else if (user_in == "--clear" || user_in == "-c") {
            system("clear");
            std::cout <<"Question: ";
            continue;
        } else if (user_in == "--help" || user_in == "-h") {
            std::string help_str = ("NAME:\n"  
            "  DeskGPT\n"
            "USAGE:\n" 
            "  DeskGPT.exe [options] <api_key>\n"
            "OPTIONS:\n"
            "  --help or -h     To see this option screen\n"
            "  --clear or -c    To Clear the screen\n"
            "  --exit           To shutdown program\n");
            std::cout << help_str <<std::endl;
            std::cout << "Question: ";
            continue;
        }      
        command =  "curl -s https://api.openai.com/v1/chat/completions \
        -H \"Authorization: Bearer "+key+"\" \
        -H \"Content-Type: application/json\" \
        -d '{ \"model\": \"gpt-3.5-turbo\", \"messages\": [{\"role\": \"user\", \"content\": \""+user_in+"\"}]}'";
        
        FILE* pipe = popen(command.c_str(),"r");  
        if (!pipe) {
            std::cout << "Failed to open pipe" << std::endl;
            return 1;
        }
        while (!feof(pipe)) {
            if (fgets(buffer, 128, pipe) != NULL){
                result_str += buffer;
            }            
        }
        //printf("%s", result_str.c_str());
        pclose(pipe);    
        rapidjson::Document document;
        rapidjson::ParseResult result = document.Parse(result_str.c_str());
        if (!result) {
            std::cout << "Error parsing JSON: " << result.Code() << std::endl;
            std::cout << result_str << std::endl;
            return 1;
        }

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
        user_in = "";
        result_str = "";
        std::cout << std::endl;
        std::cout <<"Question: ";
    }
    
}