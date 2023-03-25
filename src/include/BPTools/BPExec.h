#ifndef BPEXEC_H
#define BPEXEC_H
#include <fstream>
#include <iostream>
#include <string>


class BPExecResult {
    public:
        int exit_code;
        std::string result;
        std::string err_str;
};

class BPExec {
    public:
        static inline BPExecResult Exec(std::string cmd) {
            BPExecResult ret;
            std::string result_str = "";
            int buff_size = 1024*1024;
            char buffer[buff_size];
            FILE* pipe = popen(cmd.c_str(),"r");  
            if (!pipe) {
                ret.exit_code = 1;
                ret.err_str = "BPTools:BPExec:error - Failed to open execution pipe.";
            } else {
                while (!feof(pipe)) {
                    if (fgets(buffer, buff_size, pipe) != NULL){
                        result_str += buffer;
                    }            
                }
                ret.exit_code = 0;
                ret.result = result_str;
            }            
            pclose(pipe); 
            return ret;
        }        
};

#endif