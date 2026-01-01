#include "RestaurantProc/RestaurantProc.hpp"
#include <iostream>
#define PROC_NAME "restaurant"


int main() {
    try {
        RestaurantProc::Get().Init();
        RestaurantProc::Get().Run();
    } catch (const std::exception& e) {
        std::cerr << PROC_NAME << ": " << e.what() << std::endl;
        std::cerr << PROC_NAME << ": errno " << errno << std::endl;
    } 

    try {
        RestaurantProc::Get().Close();
    } catch (const std::exception& e) {
        std::cerr << PROC_NAME << ": " << e.what() << std::endl;
        std::cerr << PROC_NAME << ": errno " << errno << std::endl;
        return -1;
    }
    return 0;
}