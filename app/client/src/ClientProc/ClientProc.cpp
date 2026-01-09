#include "ClientProc.hpp"
#include "MessageQueue.hpp"
#include "MessageTypes/ClientMQ.hpp"
#include "PredefinedMQ.hpp"
#include <iostream>
#include <unistd.h>

ClientProc::ClientProc() { ; }
ClientProc::~ClientProc() { ; }

ClientProc& ClientProc::Get() {
    static ClientProc app;
    return app;
}

void ClientProc::Run() {
    RegisterMQMessage enterMsg;
    enterMsg.mType = RegisterMessageType::ENTER_PARK;
    enterMsg.senderPID = getpid();
    enterMsg.content = EnterPark{ .hasChild=false, .childTID=-1 };

    m_registerMQ = GetRegisterMQ(m_sharedMemory->GetData()->cashierPID, false, [this] {
        if (errno == ENOENT) {
            pLogMessage("Kasa zostala zamknieta przed wyslaniem wiadomosci!");
            return true;
        }
        return false;
    });

    bool result = m_registerMQ->SendMessage(enterMsg, [this] {
        if (errno == EBADF) {
            pLogMessage("Kasa zostala zamknieta przed wyslaniem wiadomosci!");
            return true;
        }
        return false;
    }, true, 0, 10);
    m_registerMQ = nullptr;

    if (!result)
        return;

    auto msg = m_clientQueue->RecieveMessage(true, 10);
    if (!msg)
        return;

    if (msg->mType != ClientMessageType::ENTRY_PERMIT) 
        return;

    auto reply = std::get<EntryPermit>(msg->content);

    if (!reply.allowed)
        return;

    ClientMQMessage ackMsg;
    ackMsg.content = EmptyMessage{};
    ackMsg.senderPID = getpid();
    ackMsg.mType = ClientMessageType::ACK;
    m_clientQueue->SendMessage(ackMsg);

    pLogMessage("Klient wchodzi do parku!");
}

void ClientProc::pInitImpl() {
    m_clientQueue = GetClientMQ(getpid(), true);
}

void ClientProc::pCloseImpl() {
    m_clientQueue = nullptr;
    m_registerMQ = nullptr;
}