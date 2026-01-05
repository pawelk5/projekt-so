#pragma once
#include <variant>
#include <memory>
#include "MessageQueue.hpp"


struct EntryPermit {
    bool allowed;
};

struct Bill {
    float price;
};

enum class ClientMessageType : int {
    ENTRY_PERMIT,
    BILL
};

using ClientMessageContent = std::variant<EntryPermit, Bill>;

struct ClientMQMessage {
    pid_t senderPID;
    ClientMessageType mType;
    
    ClientMessageContent content;
};

using ClientMQ = std::shared_ptr<MessageQueue<ClientMQMessage>>;