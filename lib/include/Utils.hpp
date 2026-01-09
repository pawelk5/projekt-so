#pragma once
#include <ctime>
#include <string>
#include <time.h>
#include <array>
#include <unistd.h>
#include "MessageTypes/LoggerMQ.hpp"

bool CreateEmptyFile(const std::string& filepath);
timespec CreateTimestamp(int add);

template<int N>
std::array<char, N> ToArray(const std::string& str) {
    std::array<char, N> array { 0 };
    std::size_t length = std::min((int)str.size(), N - 1);

    std::copy(str.begin(), str.begin() + length, array.begin());
    array[length] = 0;

    return array;
}

std::string CreateLogMessage(const LoggerMQMessage& msg);