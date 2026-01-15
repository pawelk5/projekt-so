#include "LoggerService.hpp"
#include "Config/Config.hpp"
#include "File.hpp"
#include "MessageTypes/LoggerMQ.hpp"
#include "PredefinedMQ.hpp"
#include <array>
#include <cerrno>
#include <exception>
#include <semaphore.h>
#include <pthread.h>
#include <iostream>
#include <stdexcept>
#include <unistd.h>

extern sem_t g_loggerInitSem;
int g_loggerStatus = 0;

void* LoggerThread(void* arg) {
    LoggerMQ mq = nullptr;
    std::array<File, 5> t_logFiles { };
    try {
        for (size_t i = 0; i < t_logFiles.size(); i++)
            t_logFiles.at(i).Open(logFileNames[i], LOGGER_FILE_FLAGS);
        mq = GetLoggerMQ(getpid(), true);

        g_loggerStatus = 1;
        sem_post(&g_loggerInitSem);
    } catch (const std::exception& e) {
        g_loggerStatus = -1;
        sem_post(&g_loggerInitSem);
        mq = nullptr;
        std::cerr << "Thread exception: " << e.what() << ", errno " << errno << std::endl;
        return nullptr;
    }

    try {
        while (true) {
            auto msg = mq->ReceiveMessage();
            if (msg->senderPID == -1)
                break;

            auto msgStr = CreateLogMessage(*msg);
            if (!t_logFiles.at(0).Write(msgStr))
                throw std::runtime_error("write error");

            if (msg->senderRole == ProcessRole::MAIN || (int)msg->senderRole >= t_logFiles.size())
                continue;

            if (!t_logFiles.at((int)msg->senderRole).Write(msgStr))
                throw std::runtime_error("write error");
        }
    } catch (const std::exception& e) {
        g_loggerStatus = -1;
        mq = nullptr;
        std::cerr << "Thread exception: " << e.what() << ", errno " << errno << std::endl;
        return nullptr;
    }

    return nullptr;
}