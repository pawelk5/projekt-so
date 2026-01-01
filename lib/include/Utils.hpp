#pragma once
#include <ctime>
#include <string>
#include <time.h>

bool CreateEmptyFile(const std::string& filepath);
timespec CreateTimestamp(int add);