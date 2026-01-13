#pragma once
#include <variant>
#include <memory>
#include "MessageQueue.hpp"
#include "SharedMessageTypes.hpp"

struct EnterAttraction {
    bool hasChild;
};

enum class AttractionMessageType : int {
    ENTER_ATTRACTION,
    EXIT_ATTRACTION
};

using AttractionMessageContent = std::variant<EnterAttraction, EmptyMessage>;

struct AttractionMQMessage {
    pid_t senderPID;
    AttractionMessageType mType;
    
    AttractionMessageContent content;
};

using AttractionMQ = std::shared_ptr<MessageQueue<AttractionMQMessage>>;