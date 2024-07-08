#pragma once

#include <string>
#include <iostream>
#include <sstream>
#include <ctime>
#include <stdarg.h>
#include <cstdio>

# define MAX_SIZE_MSG 4096

namespace ConsoleLog{
    const std::string   RESET = "\033[0m";
    const std::string   RED = "\033[31m";
    const std::string   GREEN = "\033[32m";
    const std::string   YELLOW = "\033[33m";
    const std::string   BLUE = "\033[34m";
    const std::string   MAGENTA = "\033[35m";
    const std::string   CYAN = "\033[36m";
    const std::string   WHITE = "\033[37m";

    enum Mode{
        ERROR,
        INFO,
        C_CONN,
        S_CONN,
        EXIT
    };
    void print(Mode mode, const char* format, ...);
    std::string getTimeStr();
};
