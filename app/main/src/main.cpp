#include "MainProc/MainProc.hpp"
#include <iostream>
#include <ostream>
#define PROC_NAME "main"

int main() {
    try {
        MainProc::Get().Init(true);
        MainProc::Get().Run();
    } catch (const std::exception& e) {
        std::cerr << PROC_NAME << ": " << e.what() << std::endl;
        std::cerr << PROC_NAME << ": errno " << errno << std::endl;
    } 

    try {
        MainProc::Get().Close();
    } catch (const std::exception& e) {
        std::cerr << PROC_NAME << ": " << e.what() << std::endl;
        std::cerr << PROC_NAME << ": errno " << errno << std::endl;
        return -1;
    }
    return 0;
}