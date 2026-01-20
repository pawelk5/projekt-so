#include "ClientProc.hpp"
#include "Config/Config.hpp"
#include "MessageTypes/AttractionMQ.hpp"
#include "MessageTypes/ClientMQ.hpp"
#include "MessageTypes/RegisterMQ.hpp"
#include "MessageTypes/RestaurantMQ.hpp"
#include "PredefinedMQ.hpp"
#include "SimulationData.hpp"
#include "Utils.hpp"
#include <cstdint>
#include <cstdio>
#include <ctime>
#include <string>
#include <sys/types.h>
#include <unistd.h>

bool ClientProc::pVisitRestaurant() {
    const auto ct_attractionConfig = AttractionConfig.at(RESTAURANT_INDEX);

    if (!pCreateReplyMQ(GetClientRestaurantMQ))
        return false;

    if (!pEnterRestaurant())
        return false;

    pRemoveAllMQs();

    // wejscie do atrakcji
    int attractionTime;
    if (ct_attractionConfig.canLeave)
        attractionTime = RandomInt(5, ct_attractionConfig.duration);
    else
        attractionTime = ct_attractionConfig.duration;
    
    auto attractionSem = m_semaphoreArray->GetSemaphore((uint16_t)MainSemaphoreArray::RestaurantHandler);
    pLogMessage("Klient wchodzi do restauracji " + std::string(m_enteredPark ? "przez park!" : "spoza parku!") );
    if (!attractionSem->Wait(1, false, false, attractionTime)){
        if (!m_evac)
            pLogMessage("Klient wychodzi z restauracji (timeout/interrupt)");
        else
            pLogMessage("Klient wychodzi z restauracji (ewakuacja)");
    } 
    else {
        pLogMessage("Klient wychodzi z restauracji (semop)");
    }

    pLeaveRestaurant();
    return true;
}

bool ClientProc::pEnterRestaurant() {
    RestaurantMQMessage enterMsg;
    enterMsg.mType = RestaurantMessageType::ENTER_RESTAURANT;
    enterMsg.senderPID = getpid();
    enterMsg.content = EnterRestaurant{ .hasChild=m_data.hasChild, .fromPark=m_enteredPark };

    if (!pGetRestaurantMQ(false))
        return false;

    if (!pSendRestaurantMQMessage(enterMsg, true))
        return false;
    
    m_semaphoreArray->GetSemaphore((uint16_t)MainSemaphoreArray::RestaurantEvent)->Signal();

    auto msg = m_clientQueue->ReceiveMessage(true, CLIENT_MQ_TIMEOUT);
    if (!msg)
        return false;

    if (msg->mType != ClientMessageType::ATTRACTION_ENTRY_PERMIT)
        return false;

    auto reply = std::get<AttractionEntryPermit>(msg->content);

    if (!reply.allowed)
        return false;

    ClientMQMessage ackMsg;
    ackMsg.content = EmptyMessage{};
    ackMsg.senderPID = getpid();
    ackMsg.mType = ClientMessageType::ACK;
    if (!m_clientQueue->SendMessage(ackMsg))
        return false;

    return true;
}

void ClientProc::pLeaveRestaurant() {
    RestaurantMQMessage exitMsg;
    exitMsg.mType = RestaurantMessageType::EXIT_RESTAURANT;
    exitMsg.senderPID = getpid();
    exitMsg.content = EmptyMessage{ };

    /// leaving is BLOCKING
    if (!pGetRestaurantMQ(true))
        return;

    bool result = pSendRestaurantMQMessage(exitMsg, false);
    
    m_semaphoreArray->GetSemaphore((uint16_t)MainSemaphoreArray::RestaurantEvent)->Signal();

    if (!result)
        pLogMessage("Wychodzi z restauracji, nie mogl sie polaczyc z kolejka komunikatow restauracji!");

    // dont wait for reply
    if (m_enteredPark)
        return;

    if (!pCreateReplyMQ(GetClientRestaurantMQ)) {
        pLogMessage("Wychodzi z restauracji bez odebrania rachunku (nie mogl stworzyc kolejki odpowiedzi)!");
        return;
    }
    auto msg = m_clientQueue->ReceiveMessage(true);
    if (!msg) {
        pLogMessage("Wychodzi z restauracji, nie otrzymac rachunku!");
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

    pLogMessage("Wychodzi z restauracji, placi: " + std::to_string(reply.price));
    return;
}