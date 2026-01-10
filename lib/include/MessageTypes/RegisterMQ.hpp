#pragma once
#include <variant>
#include <memory>
#include "MessageQueue.hpp"


enum class TicketType : int {
    H2,
    H4,
    H6,
    H24,    
    VIP
};

struct EnterPark {
    bool hasChild;
    bool isVip;

    TicketType ticketType;
};

struct ExitPark { 
    bool visitedRestaurant;
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