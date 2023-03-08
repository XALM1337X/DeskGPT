#include <stdlib.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include <regex>
#include "rapidjson/document.h"

int main(int argc, char **argv) {
    //TODO: Rewrite this process in a loop that waits for user input and will use the conversations session-id to carry on context with the chat model.
    //-H 'Session-Id: <your_session_id>'
    std::string pattern = "^.*\n+(.*)$";
    std::regex regex(pattern);
    std::smatch match;
    std::string key = std::string(argv[1]);
    std::string arg = std::string(argv[2]);    
    std::string command =  "curl -s https://api.openai.com/v1/chat/completions \
    -H \"Authorization: Bearer "+key+"\" \
    -H \"Content-Type: application/json\" \
    -d '{ \"model\": \"gpt-3.5-turbo\", \"messages\": [{\"role\": \"user\", \"content\": \""+arg+"\"}]}'";

    FILE* pipe = popen(command.c_str(),"r");  
    if (!pipe) {
        std::cout << "Failed to open pipe" << std::endl;
        return 1;
    }

    char buffer[128];
    std::string result_str = "";

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
                    std::string str = std::string(content.GetString());
                    if (std::regex_match(str, match, regex)) {
                        std::cout << match[1] << std::endl;
                    } else {
                        std::cout << content.GetString() << std::endl;
                    }
                }
            }
        }
    }
}