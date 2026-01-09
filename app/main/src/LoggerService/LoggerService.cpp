#include "LoggerService.hpp"
#include "MessageTypes/LoggerMQ.hpp"
#include "PredefinedMQ.hpp"
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
    try {
        mq = GetLoggerMQ(getpid(), true);

        sem_post(&g_loggerInitSem);
        g_loggerStatus = 1;
    } catch (const std::exception& e) {
        sem_post(&g_loggerInitSem);
        g_loggerStatus = -1;
        mq = nullptr;
        std::cerr << "Thread exception: " << e.what() << ", errno " << errno << std::endl;
        return nullptr;
    }

    try {
        int file = open(OUTPUT_PATH, O_WRONLY | O_TRUNC, 0600);
        if (file == -1) {
            perror("open error");
            throw std::runtime_error("open error");
        }

        while (true) {
            auto msg = mq->RecieveMessage();
            if (msg->senderPID == -1)
                break;

            auto msgStr = CreateLogMessage(*msg);
            if (write(file, msgStr.c_str(), msgStr.length()) == -1) {
                perror("write error");
                throw std::runtime_error("write error");
            }
        }

    } catch (const std::exception& e) {
        g_loggerStatus = -1;
        mq = nullptr;
        std::cerr << "Thread exception: " << e.what() << ", errno " << errno << std::endl;
        return nullptr;
    }

    return nullptr;
}