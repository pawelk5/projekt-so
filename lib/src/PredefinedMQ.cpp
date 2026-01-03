#include "PredefinedMQ.hpp"
#include "MessageQueue.hpp"
#include <string>

RegisterMQ GetRegisterMQ(pid_t cashierPID, bool cashier) {
    RegisterMQ t_mq = std::make_shared<MessageQueue<RegisterMQMessage>>();
    MessageQueueParams t_params;
    t_params.blocking = false;
    t_params.create = cashier;
    t_params.msqName = std::to_string(cashierPID) + "-cashier";
    t_params.maxMsgCount = DEFAULT_MAX_MSQ_SIZE;

    if (!t_mq->OpenMessageQueue(t_params))
        return nullptr;
    return t_mq;
}