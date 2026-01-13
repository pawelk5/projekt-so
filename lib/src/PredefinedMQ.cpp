#include "PredefinedMQ.hpp"
#include "MessageQueue.hpp"
#include "MessageTypes/AttractionMQ.hpp"
#include "MessageTypes/ClientMQ.hpp"
#include "MessageTypes/LoggerMQ.hpp"
#include <string>

RegisterMQ GetRegisterMQ(pid_t cashierPID, bool cashier, const std::function<bool()>& errorHandler, bool blocking) {
    RegisterMQ t_mq = std::make_shared<MessageQueue<RegisterMQMessage>>();
    MessageQueueParams t_params;
    t_params.blocking = blocking;
    t_params.create = cashier;
    t_params.msqName = std::to_string(cashierPID) + "-register";
    t_params.maxMsgCount = DEFAULT_MAX_MSQ_SIZE;

    if (!t_mq->OpenMessageQueue(t_params, errorHandler))
        return nullptr;
    return t_mq;
}

ClientMQ GetClientMQ(pid_t clientPID, bool client, const std::function<bool()>& errorHandler) {
    ClientMQ t_mq = std::make_shared<MessageQueue<ClientMQMessage>>();
    MessageQueueParams t_params;
    t_params.blocking = false;
    t_params.create = client;
    t_params.msqName = std::to_string(clientPID) + "-client";
    t_params.maxMsgCount = DEFAULT_MAX_MSQ_SIZE;

    if (!t_mq->OpenMessageQueue(t_params, errorHandler))
        return nullptr;
    return t_mq;
}

LoggerMQ GetLoggerMQ(pid_t loggerPID, bool logger, const std::function<bool()>& errorHandler) {
    LoggerMQ t_mq = std::make_shared<MessageQueue<LoggerMQMessage>>();
    MessageQueueParams t_params;
    t_params.blocking = true;
    t_params.create = logger;
    t_params.msqName = std::to_string(loggerPID) + "-logger";
    t_params.maxMsgCount = DEFAULT_MAX_MSQ_SIZE;

    if (!t_mq->OpenMessageQueue(t_params, errorHandler))
        return nullptr;
    return t_mq;
}

AttractionMQ GetAttractionMQ(pid_t attractionPID, bool worker, const std::function<bool()>& errorHandler, bool blocking) {
    AttractionMQ t_mq = std::make_shared<MessageQueue<AttractionMQMessage>>();
    MessageQueueParams t_params;
    t_params.blocking = true;
    t_params.create = worker;
    t_params.msqName = std::to_string(attractionPID) + "-attraction";
    t_params.maxMsgCount = DEFAULT_MAX_MSQ_SIZE;

    if (!t_mq->OpenMessageQueue(t_params, errorHandler))
        return nullptr;
    return t_mq;
}