#include "Utils.hpp"
#include <fstream>

bool CreateEmptyFile(const char* filepath) {
    std::ofstream file(filepath, std::ios::out);
    if (!file)
        return false;
    
    file.close();
    return true;
}