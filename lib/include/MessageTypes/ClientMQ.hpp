#pragma once
#include <variant>
#include <memory>
#include "IPC/MessageQueue.hpp"
#include "SharedMessageTypes.hpp"

enum class ClientMessageType : int {
    ENTRY_PERMIT,
    BILL,
    ACK
};

using ClientMessageContent = std::variant<EntryPermit, Bill, EmptyMessage>;

struct ClientMQMessage {
    pid_t senderPID;
    ClientMessageType mType;
    
    ClientMessageContent content;
};

using ClientMQ = std::shared_ptr<MessageQueue<ClientMQMessage>>;