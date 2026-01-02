#include "ClientProc/ClientProc.hpp"
#include <cerrno>
#include <iostream>
#define PROC_NAME "client"


int main() {
    try {
        ClientProc::Get().Init();
        ClientProc::Get().Run();
    } catch (const std::exception& e) {
        std::cerr << PROC_NAME << ": " << e.what() << std::endl;
        std::cerr << PROC_NAME << ": errno " << errno << std::endl;
    }

    try {
        ClientProc::Get().Close();
    } catch (const std::exception& e) {
        std::cerr << PROC_NAME << ": " << e.what() << std::endl;
        std::cerr << PROC_NAME << ": errno " << errno << std::endl;
        return -1;
    }

    return 0;
}