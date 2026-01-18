#pragma once
#include <variant>
#include <memory>
#include "IPC/MessageQueue.hpp"
#include "SharedMessageTypes.hpp"

enum class ClientMessageType : int {
    PARK_ENTRY_PERMIT,
    ATTRACTION_ENTRY_PERMIT,
    BILL,
    ACK
};

using ClientMessageContent = std::variant<ParkEntryPermit, AttractionEntryPermit, Bill, EmptyMessage>;

struct ClientMQMessage {
    pid_t senderPID;
    ClientMessageType mType;
    
    ClientMessageContent content;
};

using ClientMQ = std::shared_ptr<MessageQueue<ClientMQMessage>>;