#include "ClientProc.hpp"
#include "MessageQueue.hpp"
#include "MessageTypes/ClientMQ.hpp"
#include "MessageTypes/RegisterMQ.hpp"
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
    auto reg = GetRegisterMQ(m_sharedMemory->GetData()->cashierPID);
    RegisterMQMessage enterMsg;
    enterMsg.mType = RegisterMessageType::ENTER_PARK;
    enterMsg.senderPID = getpid();
    enterMsg.content = EnterPark{ .hasChild=false, .childTID=-1 };

    reg->SendMessage(enterMsg);

    auto msg = m_clientQueue->RecieveMessage(10);
    if (!msg)
        return;

    if (msg->mType != ClientMessageType::ENTRY_PERMIT) 
        return;

    auto reply = std::get<EntryPermit>(msg->content);
    std::cout << "Klient " << getpid() << " otrzymal wiadomosc!" << std::endl;

    if (!reply.allowed)
        return;

    ClientMQMessage ackMsg;
    ackMsg.content = EmptyMessage{};
    ackMsg.senderPID = getpid();
    ackMsg.mType = ClientMessageType::ACK;
    m_clientQueue->SendMessage(ackMsg);
}

void ClientProc::pInitImpl() {
    m_clientQueue = GetClientMQ(getpid(), true);
}

void ClientProc::pCloseImpl() {
    m_clientQueue = nullptr;
}