#pragma once
#include "Messages.hpp"
#include "MessageQueue.hpp"
#include <memory>

typedef std::shared_ptr<MessageQueue<RegisterMQMessage>> RegisterMQ;
RegisterMQ GetCashierMQ(pid_t cashierPID, bool cashier = false);