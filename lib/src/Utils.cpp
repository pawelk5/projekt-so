#include "Utils.hpp"
#include <fstream>

bool CreateEmptyFile(const std::string& filepath) {
    std::ofstream file(filepath.c_str(), std::ios::out);
    if (!file)
        return false;
    
    file.close();
    return true;
}