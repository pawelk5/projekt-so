#pragma once
#include <ctime>
#include <string>
#include <time.h>
#include <array>
#include <unistd.h>
#include "MessageTypes/LoggerMQ.hpp"

/// Creates or truncates a file
/// \param filepath path to the file
/// \returns false on failure
bool CreateEmptyFile(const std::string& filepath);

/// Creates timespec struct
/// \param add number of seconds added to current time
timespec CreateTimestamp(int add = 0);

/// Creates timespec struct
/// \param t seconds since Unix epoch
timespec CreateTimestampT(time_t t = time(NULL));

/// Returns number of seconds to a specified time
/// \param t seconds since Unix epoch
int TimeRemaining(time_t t);

/// Converts string to fixed-size char array
/// \tparam N size of output array
/// \param str string to be converted
template<int N>
std::array<char, N> ToArray(const std::string& str) {
    std::array<char, N> array { 0 };
    std::size_t length = std::min((int)str.size(), N - 1);

    std::copy(str.begin(), str.begin() + length, array.begin());
    array[length] = 0;

    return array;
}

/// Converts logger message struct to a formatted string
/// Format: [time] (pid): <message>
/// \param str string to be converted
/// \returns formatted string
std::string CreateLogMessage(const LoggerMQMessage& msg);