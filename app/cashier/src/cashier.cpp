#include "CashierProc/CashierProc.hpp"
#include <iostream>
#define PROC_NAME "cashier"


int main() {
    try {
        CashierProc::Get().Init();
        CashierProc::Get().Run();
    } catch (const std::exception& e) {
        std::cerr << PROC_NAME << ": " << e.what() << std::endl;
        std::cerr << PROC_NAME << ": errno " << errno << std::endl;
    } 

    try {
        CashierProc::Get().Close();
    } catch (const std::exception& e) {
        std::cerr << PROC_NAME << ": " << e.what() << std::endl;
        std::cerr << PROC_NAME << ": errno " << errno << std::endl;
        return -1;
    }
    return 0;
}