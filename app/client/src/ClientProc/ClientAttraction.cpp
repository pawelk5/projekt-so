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


int ClientProc::pEnterAttraction(int attractionID) {
    AttractionMQMessage enterMsg;
    enterMsg.mType = AttractionMessageType::ENTER_ATTRACTION;
    enterMsg.senderPID = getpid();
    enterMsg.content = EnterAttraction{ .hasChild=m_data.hasChild };

    if (!pGetAttractionMQ(attractionID, false))
        return -1;

    if (!pSendAttractionMQMessage(enterMsg, true))
        return -1;
    
    m_attractionMQ = nullptr;
    m_semaphoreArray->GetSemaphore((uint16_t)MainSemaphoreArray::AttractionEvent1 + attractionID)->Signal();

    auto msg = m_clientQueue->ReceiveMessage(true, CLIENT_MQ_TIMEOUT);
    if (!msg)
        return -1;

    if (msg->mType != ClientMessageType::ATTRACTION_ENTRY_PERMIT)
        return -1;

    auto reply = std::get<AttractionEntryPermit>(msg->content);

    if (!reply.allowed)
        return -1;

    ClientMQMessage ackMsg;
    ackMsg.content = EmptyMessage{};
    ackMsg.senderPID = getpid();
    ackMsg.mType = ClientMessageType::ACK;
    if (!m_clientQueue->SendMessage(ackMsg))
        return -1;

    return reply.leaveSemaphoreID;
}

void ClientProc::pLeaveAttraction(int attractionID) {
    AttractionMQMessage exitMsg;
    exitMsg.mType = AttractionMessageType::EXIT_ATTRACTION;
    exitMsg.senderPID = getpid();
    exitMsg.content = EmptyMessage{ };

    /// leaving is BLOCKING
    if (!pGetAttractionMQ(attractionID, true))
        return;

    bool result = pSendAttractionMQMessage(exitMsg, false);
    m_attractionMQ = nullptr;
    
    m_semaphoreArray->GetSemaphore((uint16_t)MainSemaphoreArray::AttractionEvent1 + attractionID)->Signal();

    if (!result)
        pLogMessage("Wychodzi z atrakcji, nie mogl sie polaczyc z kolejka komunikatow pracownika!");
}

void ClientProc::pVisitAttraction(int attractionID) {
    const auto ct_attractionConfig = AttractionConfig.at(attractionID);

    pCreateAttractionReplyMQ((uint8_t)attractionID);
    auto semID = pEnterAttraction(attractionID);
    m_clientQueue = nullptr;

    if (semID != -1) {
        // klient w atrakcji
        pLogMessage("Klient wchodzi do atrakcji " + std::to_string(attractionID));
        int attractionTime;
        if (ct_attractionConfig.canLeave)
            attractionTime = RandomInt(5, ct_attractionConfig.duration);
        else
            attractionTime = ct_attractionConfig.duration;
        auto attractionSem = m_semaphoreArray->GetSemaphore(semID);
                    
        if (!attractionSem->Wait(1, false, false, attractionTime)){
            if (!m_evac) {
                pLogMessage("Klient wychodzi z atrakcji " + std::to_string(attractionID) + " (timeout/interrupt)");
                if (pCreateAttractionReplyMQ((uint8_t)attractionID))
                    pLeaveAttraction(attractionID);
            }
            else {
                pLogMessage("Klient wychodzi z atrakcji " + std::to_string(attractionID) + " (ewakuacja)");
            }
        } else {
            pLogMessage("Klient wychodzi z atrakcji " + std::to_string(attractionID) + " (semop)");
        }
    }
}
