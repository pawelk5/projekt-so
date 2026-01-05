#pragma once
#include <variant>
#include <memory>
#include "MessageQueue.hpp"


struct EndSimulation {};

enum class MainMQMessageType : int {
    END_SIMULATION
};

using MainMQMessageContent = std::variant<EndSimulation>;

struct MainMQMessage {
    pid_t senderPID;
    MainMQMessageType mType;

    MainMQMessageContent content;
};

using MainMessageQueue = std::shared_ptr<MessageQueue<MainMQMessage>>;
