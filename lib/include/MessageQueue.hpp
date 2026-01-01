#pragma once
#include "Utils.hpp"
#include <cerrno>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <sys/types.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <stdint.h>
#include <unistd.h>
#include <iostream>
#define PARK_QUEUE_ID "/park-"
#define DEFAULT_MAX_MSQ_SIZE 10

struct MessageQueueParams {
    std::string msqName;
    uint16_t maxMsgCount;
    bool blocking;
    bool create;
};

template<class MessageType>
class MessageQueue {
public:
    MessageQueue()  
        :m_msqID(-1), m_owner(false)
    { ; }

    ~MessageQueue() { CloseMessageQueue(); }
   
    bool OpenMessageQueue(const MessageQueueParams& params) {
        if (m_msqID >= 0)
            CloseMessageQueue();

        mq_attr mqattr;
        mqattr.mq_flags = 0;
        mqattr.mq_curmsgs = 0;
        mqattr.mq_msgsize = sizeof(MessageType);
        mqattr.mq_maxmsg = params.maxMsgCount;

        m_msqName = PARK_QUEUE_ID + params.msqName;
        m_msqID = mq_open(m_msqName.c_str(), O_RDWR | O_CREAT | (params.create ? O_EXCL : 0),
            0600, &mqattr);
        
        if (m_msqID == -1)
            throw std::runtime_error("Couldn't open message queue!");

        return true;
    }
    
    bool CloseMessageQueue() {
        if (m_msqID <= 0)
            return false;

        if (mq_close(m_msqID) == -1)
            throw std::runtime_error("Couldn't unlink message queue!");

        if (m_owner) {
            if (mq_unlink(m_msqName.c_str()) == -1)
                throw std::runtime_error("Couldn't close message queue!");
        }
        
        m_msqID = 0;
        m_owner = false;

        return true;
    }

    bool SendMessage(const MessageType& msg, int priority = 0, int timeout = -1) {
        if (!m_msqID)
            return false;

        int result = 0;
        if (timeout > 0) {
            auto ts = CreateTimestamp(timeout);
            result = mq_timedsend(m_msqID, (char*)(&msg), sizeof(msg), priority, &ts);
        }
        else {
            result = mq_send(m_msqID, (char*)(&msg), sizeof(msg), priority);
        }

        if (result == -1) {
            if (errno == EAGAIN)
                return false;
            else
                throw std::runtime_error("Couldn't recieve message!");
        }

        return true;
    }

    std::shared_ptr<MessageType> RecieveMessage(int timeout = -1) {
        if (!m_msqID)
            return nullptr;

        auto dst = std::make_shared<MessageType>();
        int result = 0;
        if (timeout > 0) {
            auto ts = CreateTimestamp(timeout);
            result = mq_timedreceive(m_msqID, (char*)(&(*dst)), sizeof(int), NULL, &ts);
        }
        else {
            result = mq_receive(m_msqID, (char*)(&(*dst)), sizeof(int), NULL);
        }

        if (result == -1){
            if (errno == EAGAIN)
                return nullptr;
            else
                throw std::runtime_error("Couldn't recieve message!");
        }

        return dst;
    }

private:
    mqd_t m_msqID;

    std::string m_msqName;
    bool m_owner;
};