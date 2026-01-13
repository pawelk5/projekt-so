#pragma once
#include <variant>
#include <memory>
#include "MessageQueue.hpp"
#include "SharedMessageTypes.hpp"


enum class AttractionMessageType : int {
    ENTRY_PERMIT
};

using AttractionMessageContent = EntryPermit;

struct AttractionMQMessage {
    pid_t senderPID;
    AttractionMessageType mType;
    
    AttractionMessageContent content;
};

using AttractionMQ = std::shared_ptr<MessageQueue<AttractionMQMessage>>;