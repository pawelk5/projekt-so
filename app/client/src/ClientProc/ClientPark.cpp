#include "ClientProc.hpp"
#include "Config/Config.hpp"
#include "MessageTypes/AttractionMQ.hpp"
#include "MessageTypes/ClientMQ.hpp"
#include "MessageTypes/RegisterMQ.hpp"
#include "PredefinedMQ.hpp"
#include "SimulationData.hpp"
#include "Utils.hpp"
#include <cstdio>
#include <ctime>
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

bool ClientProc::pEnterPark() {
    if (m_enteredPark)
        pLeavePark(m_visitedRestaurant);

    RegisterMQMessage enterMsg;
    enterMsg.mType = RegisterMessageType::ENTER_PARK;
    enterMsg.senderPID = getpid();
    enterMsg.content = EnterPark{ .hasChild=m_data.hasChild, .isVip=m_data.isVip, .ticketType=m_data.ticketType };

    if (!pGetRegisterMQ(false))
        return false;

    if (!pSendRegisterMQMessage(enterMsg, true))
        return false;
    
    m_registerMQ = nullptr; 
    m_semaphoreArray->GetSemaphore((uint16_t)MainSemaphoreArray::CashierEvent)->Signal();

    auto msg = m_clientQueue->ReceiveMessage(true, CLIENT_MQ_TIMEOUT);
    if (!msg)
        return false;

    if (msg->mType != ClientMessageType::PARK_ENTRY_PERMIT)
        return false;

    auto reply = std::get<ParkEntryPermit>(msg->content);

    if (!reply.allowed)
        return false;

    ClientMQMessage ackMsg;
    ackMsg.content = EmptyMessage{};
    ackMsg.senderPID = getpid();
    ackMsg.mType = ClientMessageType::ACK;
    return m_clientQueue->SendMessage(ackMsg);
}

void ClientProc::pLeavePark(bool visitedRestaurant) {
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
    
    m_semaphoreArray->GetSemaphore((uint16_t)MainSemaphoreArray::CashierEvent)->Signal();

    if (m_data.isVip)
        return;

    if (!result || !m_clientQueue) {
        pLogMessage("Wychodzi z parku, nie mogl sie polaczyc z kolejka komunikatow kasy!");
        return;
    }

    auto msg = m_clientQueue->ReceiveMessage(true, CLIENT_MQ_TIMEOUT);
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