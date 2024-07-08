#include "consolelog.hpp"


void ConsoleLog::print(Mode mode, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    char output[MAX_SIZE_MSG];

    vsnprintf(output, MAX_SIZE_MSG, format, args);
    if (mode == ERROR)
        std::cerr << RED << getTimeStr() << "[" << "ERROR" << "]"  << " " << output << RESET << std::endl;
    else if (mode == INFO)
        std::cout << GREEN << getTimeStr() << "[" << "INFO" << "]"  << " " << output << RESET << std::endl;
    else if (mode == S_CONN)
        std::cout << YELLOW << getTimeStr() << "[" << "CONNECTION" << "]"  << " " << output << RESET << std::endl;
    else if (mode == C_CONN)
        std::cout << CYAN << getTimeStr() << "[" << "CONNECTION" << "]"  << " " << output << RESET << std::endl;
    else if (mode == EXIT)
        std::cout << MAGENTA << getTimeStr() << "[" << "EXIT" << "]"  << " " << output << RESET << std::endl;
    va_end(args);
}

std::string ConsoleLog::getTimeStr()
{
    std::time_t now = std::time(NULL);
    std::tm* localtm = std::localtime(&now);
    char buffer[256];
    std::stringstream ss("");

    std::strftime(buffer, sizeof(buffer), "%H:%M:%S", localtm);
    ss << "[" << buffer << "] ";
    return ss.str();
}
