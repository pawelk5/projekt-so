#pragma once
#include <variant>
#include <memory>
#include "MessageQueue.hpp"


struct EnterPark {
    bool hasChild;
    int childTID;
};

struct ExitPark {
    bool hasChild;
    int childTID;
};

enum class RegisterMessageType : int {
    ENTER_PARK,
    EXIT_PARK
};

using RegisterMessageContent = std::variant<EnterPark, ExitPark>;

struct RegisterMQMessage {
    pid_t senderPID;
    RegisterMessageType mType;
    
    RegisterMessageContent content;
};

using RegisterMQ = std::shared_ptr<MessageQueue<RegisterMQMessage>>;