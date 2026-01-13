#include "ClientProc.hpp"
#include "MessageQueue.hpp"
#include "MessageTypes/ClientMQ.hpp"
#include "MessageTypes/RegisterMQ.hpp"
#include "PredefinedMQ.hpp"
#include "SimulationData.hpp"
#include <cstdio>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <unistd.h>

ClientProc::ClientProc() { ; }
ClientProc::~ClientProc() { ; }

ClientProc& ClientProc::Get() {
    static ClientProc app;
    return app;
}

void ClientProc::Run() {
    if (!pCreateReplyMQ())
        return;
    m_enteredPark = pEnterPark();
    m_clientQueue = nullptr;
    if (!m_enteredPark)
        return;

    // klient jest w parku
    sleep(5);

    pCreateReplyMQ();
    pLeavePark();
    m_clientQueue = nullptr;
}

void ClientProc::pInitImpl() {
    m_enteredPark = false;

    m_data.hasChild = true;
    m_data.isVip = false;
    m_data.ticketType = TicketType::H2;
}

void ClientProc::pCloseImpl() {
    if (m_enteredPark){
        pCreateReplyMQ();
        pLeavePark();
    }

    m_clientQueue = nullptr;
    m_registerMQ = nullptr;
}

void ClientProc::pLeavePark() {
    if (!m_enteredPark)
        return;
    
    m_enteredPark = false;

    RegisterMQMessage exitMsg;
    exitMsg.mType = RegisterMessageType::EXIT_PARK;
    exitMsg.senderPID = getpid();
    exitMsg.content = ExitPark{ .visitedRestaurant=false };

    /// leaving is BLOCKING
    if (!pGetRegisterMQ(true))
        return;

    bool result = pSendRegisterMQMessage(exitMsg, false);
    m_registerMQ = nullptr;
    
    m_semaphoreArray->GetSemaphore((uint16_t)MainSemaphoreArray::CashierLoop)->Signal();

    if (!result || !m_clientQueue) {
        pLogMessage("Wychodzi z parku, nie mogl sie polaczyc z kolejka komunikatow kasy!");
        return;
    }

    auto msg = m_clientQueue->ReceiveMessage(true, 5);
    if (!msg) {
        pLogMessage("Wychodzi z parku bez odebrania rachunku (nie mogl stworzyc kolejki odpowiedzi)!");
        return;
    }

    if (msg->mType != ClientMessageType::BILL) 
        return;

    auto reply = std::get<Bill>(msg->content);

    ClientMQMessage ackMsg;
    ackMsg.content = EmptyMessage{};
    ackMsg.senderPID = getpid();
    ackMsg.mType = ClientMessageType::ACK;
    m_clientQueue->SendMessage(ackMsg, true);

    pLogMessage("Wychodzi z parku, placi: " + std::to_string(reply.price));
}

bool ClientProc::pEnterPark() {
    if (m_enteredPark)
        pLeavePark();

    RegisterMQMessage enterMsg;
    enterMsg.mType = RegisterMessageType::ENTER_PARK;
    enterMsg.senderPID = getpid();
    enterMsg.content = EnterPark{ .hasChild=m_data.hasChild, .isVip=m_data.isVip, .ticketType=m_data.ticketType };

    if (!pGetRegisterMQ(false))
        return false;

    if (!pSendRegisterMQMessage(enterMsg, true))
        return false;
    
    m_registerMQ = nullptr; 
    m_semaphoreArray->GetSemaphore((uint16_t)MainSemaphoreArray::CashierLoop)->Signal();

    auto msg = m_clientQueue->ReceiveMessage(true, 10);
    if (!msg)
        return false;

    if (msg->mType != ClientMessageType::ENTRY_PERMIT)
        return false;

    auto reply = std::get<EntryPermit>(msg->content);

    if (!reply.allowed)
        return false;

    ClientMQMessage ackMsg;
    ackMsg.content = EmptyMessage{};
    ackMsg.senderPID = getpid();
    ackMsg.mType = ClientMessageType::ACK;
    return m_clientQueue->SendMessage(ackMsg);
}

bool ClientProc::pCreateReplyMQ() {
    m_clientQueue = GetClientMQ(getpid(), true, [this] {
        if (errno == ENOSPC) {
            pLogMessage("BLAD: Za duzo kolejek komunikatow w systemie!");
            return true;
        }
        return false;
    });

    return m_clientQueue != nullptr;
}

bool ClientProc::pGetRegisterMQ(bool blocking) {
    m_registerMQ = GetRegisterMQ(m_sharedMemory->GetData()->cashierPID, false, [this] {
        if (errno == ENOENT) {
            pLogMessage("BLAD: Kasa zostala zamknieta przed wyslaniem wiadomosci!");
            return true;
        }

        return false;
    }, blocking);

    return m_registerMQ != nullptr;
}

bool ClientProc::pSendRegisterMQMessage(const RegisterMQMessage& msg, bool timeout) {
    return m_registerMQ->SendMessage(msg, [this] {
        if (errno == EBADF) {
            pLogMessage("BLAD: Kasa zostala zamknieta przed wyslaniem wiadomosci!");
            return true;
        }
        return false;
    }, true, 0, timeout ? 10 : -1);    
}