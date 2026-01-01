#include "AttractionProc/AttractionProc.hpp"
#include <iostream>
#define PROC_NAME "attraction"

int main() {
    try {
        AttractionProc::Get().Init();
        AttractionProc::Get().Run();
    } catch (const std::exception& e) {
        std::cerr << PROC_NAME << ": " << e.what() << std::endl;
        std::cerr << PROC_NAME << ": errno " << errno << std::endl;
    } 

    try {
        AttractionProc::Get().Close();
    } catch (const std::exception& e) {
        std::cerr << PROC_NAME << ": " << e.what() << std::endl;
        std::cerr << PROC_NAME << ": errno " << errno << std::endl;
        return -1;
    }
    return 0;
}