#include "RestaurantProc/RestaurantProc.hpp"
#include <iostream>

int main() {
    try {
        RestaurantProc::Get().Init();
        RestaurantProc::Get().Run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    } 

    try {
        RestaurantProc::Get().Close();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }
    return 0;
}