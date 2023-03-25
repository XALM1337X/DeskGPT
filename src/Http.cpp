#include "Http.h"
#include <regex>

HttpMessage::HttpMessage() {
    this->req_line = "";
}

HttpMessage::~HttpMessage() {

}

void HttpMessage::Parse(std::vector<std::string> lines) {
    //TODO: Decode HTTP request
}

std::string HttpMessage::HTTPDateFormatGet() {
    return "";
}