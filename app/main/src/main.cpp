#include "MainProc/MainProc.hpp"
#include <iostream>
#include <ostream>

int main() {
    try {
        MainProc::Get().Init(true);
        MainProc::Get().Run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    } 

    try {
        MainProc::Get().Close();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }
    return 0;
}