#pragma once
#include <sys/types.h>
#include <array>
#include <memory>

template<class T>
class MessageQueue;

#define LOGGER_PATH "/tmp/park-logger"
#define OUTPUT_PATH "log.txt"
#define LOGGER_MESSAGE_MAX_LENGTH 256

struct LoggerMQMessage {
    pid_t senderPID;

    time_t timestamp;
    std::array<char, LOGGER_MESSAGE_MAX_LENGTH> message;
};

using LoggerMQ = std::shared_ptr<MessageQueue<LoggerMQMessage>>;