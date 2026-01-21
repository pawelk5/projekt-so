#pragma once
#include <variant>
#include <memory>
#include "IPC/MessageQueue.hpp"
#include "SharedMessageTypes.hpp"
#include "SimulationData.hpp"

struct EnterAttraction {
    bool hasChild;

    PersonData personData;
    PersonData childData;
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