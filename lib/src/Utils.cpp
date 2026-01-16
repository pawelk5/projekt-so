#include "Utils.hpp"
#include "IPC/SemaphoreArray.hpp"
#include <random>
#include <cstddef>
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

int TimeRemaining(time_t t) {
    return t - time(NULL);
}

bool CreateProcess(const char* procName, const char* path) {
    switch (fork()) {
    case -1:
        perror("fork error!");
        throw std::runtime_error("fork error!");
        return false;
    case 0:
        if (execl(((std::string)path + procName).c_str(), procName, NULL) == -1) {
            perror("exec error!");
            exit(-1);
        }
    }
    return true;
}

bool RandomChance(float prob) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::bernoulli_distribution dist(prob);
    return dist(gen);
}

int RandomInt(int min, int max) {
    if (min > max)
        return 0;

    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(min, max);

    return dist(gen);
}