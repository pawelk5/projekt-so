#include "AttractionProc/AttractionProc.hpp"
#include <iostream>

int main() {
    try {
        AttractionProc::Get().Init();
        AttractionProc::Get().Run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    } 

    try {
        AttractionProc::Get().Close();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }
    return 0;
}