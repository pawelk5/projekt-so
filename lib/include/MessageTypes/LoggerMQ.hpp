#pragma once
#include <sys/types.h>
#include <array>
#include <memory>
#include "Config/Config.hpp"

template<class T>
class MessageQueue;

struct LoggerMQMessage {
    pid_t senderPID;

    time_t timestamp;
    std::array<char, LOGGER_MESSAGE_MAX_LENGTH> message;
};

using LoggerMQ = std::shared_ptr<MessageQueue<LoggerMQMessage>>;