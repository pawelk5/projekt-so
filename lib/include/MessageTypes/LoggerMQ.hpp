#pragma once
#include <sys/types.h>
#include <array>
#include <memory>
#include "Config/Config.hpp"

template<class T>
class MessageQueue;

enum class ProcessRole {
    MAIN,
    CASHIER,
    ATTRACTION,
    RESTAURANT,
    CLIENT
};

struct LoggerMQMessage {
    pid_t senderPID;
    ProcessRole senderRole;

    time_t timestamp;
    std::array<char, LOGGER_MESSAGE_MAX_LENGTH> message;
};

using LoggerMQ = std::shared_ptr<MessageQueue<LoggerMQMessage>>;