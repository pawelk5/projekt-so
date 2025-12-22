#include "CashierProc/CashierProc.hpp"
#include <iostream>

int main() {
    try {
        CashierProc::Get().Init();
        CashierProc::Get().Run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    } 

    try {
        CashierProc::Get().Close();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }
    return 0;
}