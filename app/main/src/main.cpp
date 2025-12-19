#include "MainProc/MainProc.hpp"
#include <iostream>

int main() {
    try {
        MainProc::Get().Init(true);
        MainProc::Get().Run();
    } catch (std::exception e) {
        perror(e.what());
    } 

    try {
        MainProc::Get().Close();
    } catch (std::exception e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }
    return 0;
}