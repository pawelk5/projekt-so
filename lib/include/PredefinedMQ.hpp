#pragma once
#include "Messages.hpp"
#include "MessageQueue.hpp"
#include <memory>

using RegisterMQ = std::shared_ptr<MessageQueue<RegisterMQMessage>>;
RegisterMQ GetRegisterMQ(pid_t cashierPID, bool cashier = false);