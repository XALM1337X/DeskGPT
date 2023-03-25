#ifndef HTTP_H
#define HTTP_H
#include <unordered_map>
#include <string>
#include <vector>
class HttpMessage {
    public:
        HttpMessage();
        ~HttpMessage();
        void Parse(std::vector<std::string>);
        //TODO: Add method to make this system call for the date:  date -u +"%a, %d %b %Y %H:%M:%S GMT"
        std::string HTTPDateFormatGet();
    
    public:
        std::string req_line;
        std::unordered_map<std::string, std::string> header_map;        
};
#endif