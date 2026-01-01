#include "ManagerProc/ManagerProc.hpp"
#include <cerrno>
#include <iostream>
#define PROC_NAME "manager"


int main() {
    try {
        ManagerProc::Get().Init();
        ManagerProc::Get().Run();
    } catch (const std::exception& e) {
        std::cerr << PROC_NAME << ": " << e.what() << std::endl;
        std::cerr << PROC_NAME << ": errno " << errno << std::endl;
    }

    try {
        ManagerProc::Get().Close();
    } catch (const std::exception& e) {
        std::cerr << PROC_NAME << ": " << e.what() << std::endl;
        std::cerr << PROC_NAME << ": errno " << errno << std::endl;
        return -1;
    }

    return 0;
}