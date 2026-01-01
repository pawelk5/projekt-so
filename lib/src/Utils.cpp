#include "Utils.hpp"
#include <ctime>
#include <fstream>

bool CreateEmptyFile(const std::string& filepath) {
    std::ofstream file(filepath.c_str(), std::ios::out);
    if (!file)
        return false;
    
    file.close();
    return true;
}

timespec CreateTimestamp(int add = 0) {
    return timespec{ .tv_sec = time(NULL) + add , .tv_nsec = 0 };
}