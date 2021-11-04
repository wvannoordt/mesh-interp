#ifndef CALL_ERROR_H
#define CALL_ERROR_H
#include <iostream>

#define CallError(mymsg) CallError_M(mymsg, __FILE__, __LINE__)

static void CallError_M(std::string message, const char* file, int line)
{
    std::string boldRed = "\033[1;31m";
    std::string reset = "\033[0m";
    std::cout << boldRed;
    std::cout << "------------------------------------------------------" << std::endl;
    std::cout << "Error: " << reset << message << std::endl;
    std::cout << boldRed << "File: " << reset << file << std::endl;
    std::cout << boldRed << "Line: " << reset << line << std::endl;
    std::cout << boldRed;
    std::cout << "------------------------------------------------------" << std::endl;
    std::cout << reset;
    abort();
}

#endif