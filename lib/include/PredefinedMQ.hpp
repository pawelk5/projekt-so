#pragma once
#include "Messages.hpp"
#include "MessageQueue.hpp"
#include <memory>

typedef std::shared_ptr<MessageQueue<RegisterMQMessage>> RegisterMQ;
RegisterMQ GetRegisterMQ(pid_t cashierPID, bool cashier = false);