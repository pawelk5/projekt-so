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

struct EntryPermit {
    bool allowed;
};

struct Bill {
    float price;
};

enum class RegisterMessageType : int {
    ENTER_PARK,
    EXIT_PARK,
    ENTRY_PERMIT,
    BILL
};

using RegisterMessageContent = std::variant<EnterPark, ExitPark, EntryPermit, Bill>;

struct RegisterMQMessage {
    int senderPID;
    RegisterMessageType mType;
    
    RegisterMessageContent content;
};

using RegisterMQ = std::shared_ptr<MessageQueue<RegisterMQMessage>>;