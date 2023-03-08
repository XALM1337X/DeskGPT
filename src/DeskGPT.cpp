#include <stdlib.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include "rapidjson/document.h"
#include <regex>

int main(int argc, char **argv) {
    std::string pattern = "^.*\n+(.*)$";
    std::regex regex(pattern);
    std::smatch match;
    std::string arg = std::string(argv[1]);
    std::string command =  "curl -s https://api.openai.com/v1/completions -H \"Content-Type: application/json\" -H \"Authorization: Bearer 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\" -d '{\"model\": \"text-davinci-003\", \"prompt\": \""+arg+"\", \"temperature\": 0, \"max_tokens\": 2048}'";
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
            const rapidjson::Value& text = choice["text"];
            if (text.IsString()) {
                std::string str = std::string(text.GetString());
                if (std::regex_match(str, match, regex)) {
                    std::cout << match[1] << std::endl;
                } else {
                    std::cout << text.GetString() << std::endl;
                }
            }
        }
    }
}

