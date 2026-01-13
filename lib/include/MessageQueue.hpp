#pragma once
#include "Utils.hpp"
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <memory>
#include <stdexcept>
#include <sys/types.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <stdint.h>
#include <unistd.h>

#define PARK_QUEUE_ID "/park-"
/// in ubuntu-based systems default max queue size is 10
#define DEFAULT_MAX_MSQ_SIZE 10

/// Struct containing message queue parameters
/// \param msqName message queue name. cannot contain '/' symbol
/// \param maxMsgCount max number of messages queued in the message queue
/// \param blocking if true, message queue will work in blocking mode
/// \param create if true, message queue object will be responsible for creating and removing the message queue
struct MessageQueueParams {
    std::string msqName;
    uint16_t maxMsgCount;
    bool blocking;
    bool create;
};

/// Message queue class template
/// Automatically handles detaching and removing message queues
/// Provides methods to send and receive messages of type MessageType
template<class MessageType>
class MessageQueue {
public:
    MessageQueue()  
        :m_msqID(-1), m_owner(false)
    { ; }

    ~MessageQueue() { CloseMessageQueue(); }
    
    /// Creates or attaches client message queue
    /// \param params structure containing message queue parameters
    /// \param errorHandler function that allows to additionally handle certain errors. the function should return false if it fails to handle any error
    bool OpenMessageQueue(const MessageQueueParams& params, const std::function<bool()>& errorHandler) {
        if (m_msqID >= 0)
            CloseMessageQueue();

        mq_attr mqattr;
        mqattr.mq_flags = O_RDWR;
        mqattr.mq_curmsgs = 0;
        mqattr.mq_msgsize = sizeof(MessageType);
        mqattr.mq_maxmsg = params.maxMsgCount;

        m_msqName = PARK_QUEUE_ID + params.msqName;
        m_msqID = mq_open(m_msqName.c_str(), O_RDWR | (params.create ? O_CREAT | O_EXCL : 0),
            0600, &mqattr);
        
        if (m_msqID == -1) {
            if (errorHandler())
                return false;
            perror("mq_open error");
            throw std::runtime_error("Couldn't open message queue!");
            return false;
        }

        m_owner = params.create;
        
        return true;
    }

    /// Creates or attaches client message queue
    /// \param params structure containing message queue parameters
    bool OpenMessageQueue(const MessageQueueParams& params) {
        return OpenMessageQueue(params, [] { return false; });
    }
    
    /// Detachs the message queue and deletes it (if create flag was passed in message queue params)
    /// \returns false if object does not hold any message queue
    /// \throws std::runtime_error if detaching fails
    bool CloseMessageQueue() {
        if (m_msqID <= 0)
            return false;

        if (mq_close(m_msqID) == -1) {
            perror("mq_close error");
            throw std::runtime_error("Couldn't unlink message queue!");
            return false;
        }

        if (m_owner) {
            if (mq_unlink(m_msqName.c_str()) == -1) {
                perror("mq_unlink error");
                throw std::runtime_error("Couldn't close message queue!");
                return false;
            }
        }
        
        m_msqID = 0;
        m_owner = false;

        return true;
    }

    /// Send message using message queue
    /// \param msg message to be sent
    /// \param errorHandler function that allows to additionally handle certain errors. the function should return false if it fails to handle any error
    /// \param retryOnInterrupt if the function is interrupted by a signal the function will retry to send the message
    /// \param priority message priority
    /// \param timeout timeout in seconds. if equal to -1 the function will wait (in blocking mode) or work in non-blocking manner
    /// \returns false if message queue is not initialized or timed out/interrupted or message was handled by errorHandler
    /// \throws if error occurs while sending a message and is not handled by errorHandler
    bool SendMessage(const MessageType& msg, const std::function<bool()>& errorHandler, bool retryOnInterrupt = true, int priority = 0, int timeout = -1) {
        if (!m_msqID)
            return false;

        int result = 0;
        do {
            if (timeout >= 0) {
                auto ts = CreateTimestamp(timeout);
                result = mq_timedsend(m_msqID, (char*)(&msg), sizeof(msg), priority, &ts);
            }
            else {
                result = mq_send(m_msqID, (char*)(&msg), sizeof(msg), priority);
            }
        } while (retryOnInterrupt && result == -1 && errno == EINTR);

        if (result != -1) 
            return true;

        if (errno == EAGAIN || errno == ETIMEDOUT || errno == EINTR)
            return false;

        if (errorHandler())
            return false;

        perror("mq_send/mq_timedsend error");
        throw std::runtime_error("Couldn't send message!");

        return false;
    }

    /// Send message using message queue
    /// \param msg message to be sent
    /// \param retryOnInterrupt if the function is interrupted by a signal the function will retry to send the message
    /// \param priority message priority
    /// \param timeout timeout in seconds. if equal to -1 the function will wait (in blocking mode) or work in non-blocking manner
    /// \returns false if message queue is not initialized or timed out/interrupted or message was handled by errorHandler
    /// \throws if error occurs while sending a message and is not handled by errorHandler
    bool SendMessage(const MessageType& msg, bool retryOnInterrupt = true, int priority = 0, int timeout = -1) {
        return SendMessage(msg, [] { return false; }, retryOnInterrupt, priority, timeout);
    }

    /// Receive message using message queue
    /// \param retryOnInterrupt if the function is interrupted by a signal the function will retry to receive the message
    /// \param timeout timeout in seconds. if equal to -1 the function will wait (in blocking mode) or work in non-blocking manner
    /// \returns pointer to message or nullptr if timed out/interrupted
    /// \throws if error occurs while receiving a message
    std::shared_ptr<MessageType> ReceiveMessage(bool retryOnInterrupt = true, int timeout = -1) {
        if (!m_msqID)
            return nullptr;

        auto dst = std::make_shared<MessageType>();
        int result = 0;
        
        do {
            if (timeout > 0) {
                auto ts = CreateTimestamp(timeout);
                result = mq_timedreceive(m_msqID, (char*)(&(*dst)), sizeof(MessageType), NULL, &ts);
            }
            else {
                result = mq_receive(m_msqID, (char*)(&(*dst)), sizeof(MessageType), NULL);
            }
        } while (retryOnInterrupt && result == -1 && errno == EINTR);

        if (result == -1){
            if (errno == EAGAIN || errno == ETIMEDOUT)
                return nullptr;
            else {
                perror("mq_receive/mq_timedreceive error");
                throw std::runtime_error("Couldn't recieve message!");
            }
        }

        return dst;
    }

private:
    mqd_t m_msqID;

    std::string m_msqName;
    bool m_owner;
};