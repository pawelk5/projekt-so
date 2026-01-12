#include "Utils.hpp"
#include "SemaphoreArray.hpp"
#include <ctime>
#include <fcntl.h>
#include <sys/stat.h>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <unistd.h>

bool CreateEmptyFile(const std::string& filepath) {
    int file = open(filepath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (file == -1) {
        perror("open error");
        return false;
    }
    
    close(file);
    return true;
}

timespec CreateTimestamp(int add) {
    return timespec{ .tv_sec = time(NULL) + add , .tv_nsec = 0 };
}

timespec CreateTimestampT(time_t t) {
    return timespec{ .tv_sec = t , .tv_nsec = 0 };
}

std::string CreateLogMessage(const LoggerMQMessage& message) {
    std::tm localtime;
    localtime_r(&message.timestamp, &localtime);

    std::stringstream msg;
    msg << "[" << std::put_time(&localtime, "%Y-%m-%d %H:%M:%S") << "] " << "(" << message.senderPID << "): ";
    msg << message.message.data() << "\n";
    return msg.str();
}