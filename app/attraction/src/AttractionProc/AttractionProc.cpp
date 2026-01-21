#include "AttractionProc.hpp"
#include "AttractionHandler.hpp"
#include "IPC/Signal.hpp"
#include "MessageTypes/AttractionMQ.hpp"
#include "MessageTypes/LoggerMQ.hpp"
#include "MessageTypes/SharedMessageTypes.hpp"
#include "PredefinedMQ.hpp"
#include "SimulationData.hpp"
#include "Utils.hpp"
#include <cstdint>
#include <memory>
#include <string>
#include <iostream>

static volatile bool paused = false;

void SigUsr1(int sig) {
    paused = true;
    AttractionProc::Get().CloseAttraction();
}

void SigUsr2(int sig) {
    paused = false;
    AttractionProc::Get().OpenAttraction();
}

void AttractionProc::CloseAttraction() {
    pLogMessage("Zamykanie atrakcji " + std::to_string(GetAttractionID()) + "!");
    for (auto it = m_attractionHandlers.begin(); it != m_attractionHandlers.end(); ++it)
        it->second = nullptr;
}

void AttractionProc::OpenAttraction() {
    pLogMessage("Otwieranie atrakcji " + std::to_string(GetAttractionID()) + "!");
}

AttractionProc::AttractionProc()
    :m_attractionID(-1)
{ 
    ;
}

AttractionProc::~AttractionProc() { ; }

AttractionProc& AttractionProc::Get() {
    static AttractionProc app;
    return app;
}

void AttractionProc::pInitImpl() {
    CreateSignalHandler(SIGUSR1, SigUsr1);
    CreateSignalHandler(SIGUSR2, SigUsr2);

    m_sharedMemory->GetSemLock().Execute([this]() {
        if (!m_sharedMemory->GetData()->isOpen)
            throw std::runtime_error("park is closed!");

        bool changed = false;
        for (int i = 0; i < ATTRACTION_COUNT; i++) {
            if (m_sharedMemory->GetData()->attractionPID.at(i) == 0) {
                m_sharedMemory->GetData()->attractionPID.at(i) = getpid();
                changed = true;
                m_attractionID = i;
                break;
            }
        }
        if (!changed)
            throw std::runtime_error("all atractions already exist!");
    });

    m_eventSemaphore = m_semaphoreArray->GetSemaphore(
        (uint16_t) MainSemaphoreArray::AttractionEvent1 + GetAttractionID());

    cm_attractionConfig = AttractionConfig.at(GetAttractionID());

    for (int i = 0; i < cm_attractionConfig.handlerCount; i++)
        m_attractionHandlers[GetFirstHandlerSemaphoreID(GetAttractionID()) + i] = nullptr;

    m_replyMQ = nullptr;
    m_attractionMQ = GetAttractionMQ(getpid(), true);
    if (!m_attractionMQ)
        throw std::runtime_error("Couldn't create attraction message queue!");

    pSetProcessRole(ProcessRole::ATTRACTION);

    pLogMessage("Atrakcja " + std::to_string(GetAttractionID()) + " rozpoczyna prace!");
}

void AttractionProc::Run() {
    while (m_sharedMemory->GetData()->isOpen) {
        auto timeout = pGetNextTimeout();

        m_eventSemaphore->Wait(1, false, false, timeout);
        for (auto& handler : m_attractionHandlers) {
            if (handler.second) {
                if (handler.second->Finished())
                    handler.second = nullptr;
            }
        }
        pHandleAttraction();
    }
}

void AttractionProc::pHandleAttraction() {
    if (paused)
        return;

    pHandleAttractionMQ();

    if (m_enterQueue.size() > cm_attractionConfig.maxClientsPerHandler || m_attractionMQ->GetMessageCount() == 0){
        while (pCreateNewHandler()) { ; }
    }
}

void AttractionProc::pHandleAttractionMQ() {
    auto clientMsg = m_attractionMQ->ReceiveMessage(true, DEFAULT_MQ_TIMEOUT);
    if (!clientMsg)
        return;

    auto replyPID = clientMsg->senderPID;

    try {
        switch (clientMsg->mType) {
        case AttractionMessageType::ENTER_ATTRACTION:
            pHandleEnterAttraction(*clientMsg);
            break;

        case AttractionMessageType::EXIT_ATTRACTION:
            pRemoveClient(replyPID);
            break;
        default:
            std::cerr << "Zly typ wiadomosci!" << std::endl;
        }
    } catch (std::bad_variant_access variant_error) {
        std::cerr << "Zla zawartosc wiadomosci!\n" << variant_error.what() << std::endl;
    }
}

void AttractionProc::pRemoveClient(pid_t pid) {
    for (auto& handler : m_attractionHandlers) {
        if (!handler.second)
            continue;

        if (handler.second->RemoveClient(pid))
            break;
    }
}

bool AttractionProc::pCreateReplyMQ(pid_t pid) {
    m_replyMQ = GetClientAttractionMQ(pid, GetAttractionID(), false, [this, pid] {
        if (errno == ENOENT)
            return true;
        return false;
    });

    return m_replyMQ != nullptr;
}

bool AttractionProc::pCreateNewHandler() {
    if (paused)
        return false;
    
    int newHandlerID = -1;
    for (auto& handler : m_attractionHandlers) {
        if (!handler.second) {
            newHandlerID = handler.first;
            break;
        }            
    }
    if (newHandlerID == -1)
        return false;

    AttractionHandlerData newHandlerData {
        .leaveSemaphore = m_semaphoreArray->GetSemaphore(newHandlerID),
        .maxCientCount = cm_attractionConfig.maxClientsPerHandler,
        .attractionDuration = cm_attractionConfig.duration,
        .isRestaurant = false
    };

    auto newHandler = std::make_shared<AttractionHandler>(newHandlerData);

    while (m_enterQueue.size() > 0) {
        if (!pRegisterClient(m_enterQueue[0], newHandler))
            break;
        m_enterQueue.erase(m_enterQueue.begin());
    }

    if (!newHandler->IsEmpty()) {
        m_attractionHandlers[newHandlerID] = newHandler;
        m_attractionHandlers[newHandlerID]->StartAttraction();
        pLogMessage("Atrakcja " +
             std::to_string(GetAttractionID()) + " rozpoczyna prace z " 
             + std::to_string(m_attractionHandlers[newHandlerID]->GetClientCount()) + "/" +
              std::to_string(cm_attractionConfig.maxClientsPerHandler) + " klientami!");
        return true;
    }

    return false;
}

bool AttractionProc::pRegisterClient(const AttractionMQMessage& message, std::shared_ptr<AttractionHandler> handler) {
    auto replyPID = message.senderPID;
    try {
        auto msgContent = std::get<EnterAttraction>(message.content);
        bool allowed = m_sharedMemory->GetData()->isOpen && !paused;

        if (allowed) {
            allowed = MeetsAttractionCriteria(GetAttractionID(), 
                msgContent.personData, msgContent.childData, msgContent.hasChild);
        }

        if ((handler->GetClientCount() + 1 + msgContent.hasChild > cm_attractionConfig.maxClientsPerHandler)
            && allowed)
            return false;

        ClientMQMessage replyMsg;
        replyMsg.senderPID = getpid();
        replyMsg.mType = ClientMessageType::ATTRACTION_ENTRY_PERMIT;

        replyMsg.content = AttractionEntryPermit{ .allowed = allowed, .leaveSemaphoreID = (uint16_t)handler->GetHandlerData().leaveSemaphore->GetSemaphoreID() };

        if (!pCreateReplyMQ(replyPID))
            return true;

        if (!pSendReply(replyPID, replyMsg))
            return true;

        // dont wait for ack message
        if (!allowed)
            return true;

        auto msg = m_replyMQ->ReceiveMessage(true, DEFAULT_MQ_TIMEOUT);

        // no ack message
        if (!msg)
            return true;

        if (msg->mType == ClientMessageType::ACK) {
            // TODO
            pLogMessage("Klient " + std::to_string(replyPID) + " wchodzi do atrakcji " + std::to_string(GetAttractionID()) + "!");
            handler->AddClient(replyPID, msgContent.hasChild);
        }
    } catch (const std::exception& e) {
        // client left the queue before response
        pLogMessage("Przy obsludze klienta " + std::to_string(replyPID) + " nastapil blad w komunikacji!");
        pLogMessage((std::string)"BLAD: " + e.what());
    }

    m_replyMQ = nullptr;
    return true;
}

void AttractionProc::pHandleEnterAttraction(const AttractionMQMessage& message) {
    m_enterQueue.push_back(message);
}

bool AttractionProc::pSendReply(pid_t pid, const ClientMQMessage& msg) {
    return m_replyMQ->SendMessage(msg,
        [this, pid] {
            if (errno == EBADF) {
                pLogMessage("Klient " + std::to_string(pid) + " opuscil kolejke przed odebraniem wiadomosci!");
                return true;
            }
            return false;
        }, true, 0, DEFAULT_MQ_TIMEOUT);
}

void AttractionProc::pCloseImpl() {
    if (GetAttractionID() != -1)
        m_sharedMemory->GetSemLock().Execute([this]() {
            m_sharedMemory->GetData()->attractionPID.at(GetAttractionID()) = 0;
        });

    m_attractionMQ = nullptr;
    m_replyMQ = nullptr;
    pCloseHandlers();
    pLogMessage("Atrakcja " + std::to_string(m_attractionID) + " konczy prace!");
}

int AttractionProc::GetAttractionID() {
    return m_attractionID;
}

time_t AttractionProc::pGetNextTimeout() {
    time_t nextTimeout = -1;
    for (auto& handler : m_attractionHandlers) {
        if (!handler.second)
            continue;

        const auto timeout = handler.second->GetAttractionFinishTime();

        if (timeout == 0)
            continue;
        if (nextTimeout == -1)
            nextTimeout = timeout;
        else if (timeout < nextTimeout)
            nextTimeout = timeout;
    }

    if (nextTimeout != -1)
        return nextTimeout - time(NULL);

    return nextTimeout;
}

void AttractionProc::pCloseHandlers() {
    for (auto& handler : m_attractionHandlers)
        handler.second = nullptr;
}