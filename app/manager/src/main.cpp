#include "ManagerProc/ManagerProc.hpp"
#include <iostream>

int main() {
    try {
        ManagerProc::Get().Init();
        ManagerProc::Get().Run();
    } catch (std::exception e) {
        std::cerr << e.what() << std::endl;
    }

    try {
        ManagerProc::Get().Close();
    } catch (std::exception e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }

    return 0;
}