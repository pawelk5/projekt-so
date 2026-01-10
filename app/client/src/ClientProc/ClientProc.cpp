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

    RegisterMQMessage enterMsg;
    enterMsg.mType = RegisterMessageType::EXIT_PARK;
    enterMsg.senderPID = getpid();
    enterMsg.content = ExitPark{ .visitedRestaurant=false };

    /// leaving is BLOCKING
    m_registerMQ = GetRegisterMQ(m_sharedMemory->GetData()->cashierPID, false, [this] {
        if (errno == ENOENT) {
            pLogMessage("Kasa zostala zamknieta przed wyslaniem wiadomosci!");
            return true;
        }
        return false;
    }, true);

    if (!m_registerMQ)
        return;

    bool result = m_registerMQ->SendMessage(enterMsg, [this] {
        if (errno == EBADF) {
            pLogMessage("Kasa zostala zamknieta przed wyslaniem wiadomosci!");
            return true;
        }
        
        return false;
    }, true);
    m_registerMQ = nullptr;
    
    m_semaphoreArray->GetSemaphore((u_int16_t)MainSemaphoreArray::CashierLoop)->Signal();

    if (!result || !m_clientQueue)
        return;

    auto msg = m_clientQueue->RecieveMessage(true, 10);
    if (!msg)
        return;

    if (msg->mType != ClientMessageType::BILL) 
        return;

    auto reply = std::get<Bill>(msg->content);

    ClientMQMessage ackMsg;
    ackMsg.content = EmptyMessage{};
    ackMsg.senderPID = getpid();
    ackMsg.mType = ClientMessageType::ACK;
    m_clientQueue->SendMessage(ackMsg);

    pLogMessage("Wychodzi z parku, placi: " + std::to_string(reply.price));
}

bool ClientProc::pEnterPark() {
    if (m_enteredPark)
        pLeavePark();

    RegisterMQMessage enterMsg;
    enterMsg.mType = RegisterMessageType::ENTER_PARK;
    enterMsg.senderPID = getpid();
    enterMsg.content = EnterPark{ .hasChild=m_data.hasChild, .isVip=m_data.isVip, .ticketType=m_data.ticketType };

    m_registerMQ = GetRegisterMQ(m_sharedMemory->GetData()->cashierPID, false, [this] {
        if (errno == ENOENT) {
            pLogMessage("BLAD: Kasa zostala zamknieta przed wyslaniem wiadomosci!");
            return true;
        }

        return false;
    });

    bool result = m_registerMQ->SendMessage(enterMsg, [this] {
        if (errno == EBADF) {
            pLogMessage("BLAD: Kasa zostala zamknieta przed wyslaniem wiadomosci!");
            return true;
        }
        return false;
    }, true, 0, 10);
    m_registerMQ = nullptr;
    m_semaphoreArray->GetSemaphore((u_int16_t)MainSemaphoreArray::CashierLoop)->Signal();

    if (!result) {
        return false;
    }

    auto msg = m_clientQueue->RecieveMessage(true, 10);
    if (!msg) {

        return false;
    }
    if (msg->mType != ClientMessageType::ENTRY_PERMIT) {
        return false;
    }

    auto reply = std::get<EntryPermit>(msg->content);

    if (!reply.allowed) {
        return false;
    }

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