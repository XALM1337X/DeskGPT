#ifndef BPSTRING_H
#define BPSTRING_H
#include <vector>
#include <string>

class BPStrings {
    public:
        static inline std::vector<std::string> SplitString(const std::string inputString, char delimiter) {
            std::vector<std::string> substrings;
            size_t start = 0;
            size_t end = inputString.find(delimiter);
            while (end != std::string::npos) {
                substrings.push_back(inputString.substr(start, end - start));
                start = end + 1;
                end = inputString.find(delimiter, start);
            }
            substrings.push_back(inputString.substr(start));
            return substrings;
        }
};

#endif