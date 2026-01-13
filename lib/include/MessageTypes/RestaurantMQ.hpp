#pragma once
#include <variant>
#include <memory>
#include "IPC/MessageQueue.hpp"
#include "SharedMessageTypes.hpp"

struct EnterRestaurant {
    bool hasChild;
    
    bool fromPark;
};

enum class RestaurantMessageType : int {
    ENTER_RESTAURANT,
    EXIT_RESTAURANT
};

using RestaurantMessageContent = std::variant<EnterRestaurant, EmptyMessage>;

struct RestaurantMQMessage {
    pid_t senderPID;
    RestaurantMessageType mType;
    
    RestaurantMessageContent content;
};

using RestaurantMQ = std::shared_ptr<MessageQueue<RestaurantMQMessage>>;