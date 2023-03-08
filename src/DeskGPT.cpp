#include <iostream>
#include <stdlib.h>

int main(int argc, char **argv) {
    std::string arg = std::string(argv[1]);
    std::string command =  "curl https://api.openai.com/v1/completions -H \"Content-Type: application/json\" -H \"Authorization: Bearer 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\" -d '{\"model\": \"text-davinci-003\", \"prompt\": \""+arg+"\", \"temperature\": 0, \"max_tokens\": 2048}'";
    std::cout << std::endl;
    system(command.c_str());  
}

//