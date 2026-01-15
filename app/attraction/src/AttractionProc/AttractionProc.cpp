#include "AttractionProc.hpp"
#include "IPC/Signal.hpp"
#include "MessageTypes/AttractionMQ.hpp"
#include "MessageTypes/LoggerMQ.hpp"
#include "PredefinedMQ.hpp"
#include "SimulationData.hpp"
#include <string>
#include <iostream>

static volatile bool paused = false;

void SigUsr1(int sig) {
    paused = true;
    AttractionProc::Get().CloseAttraction();
}

void SigUsr2(int sig) {
    AttractionProc::Get().OpenAttraction();
}

void AttractionProc::CloseAttraction() {
    pLogMessage("Zamykanie atrakcji " + std::to_string(GetAttractionID()) + "!");
    m_pauseSemaphore->SetValue(0);
}

void AttractionProc::OpenAttraction() {
    pLogMessage("Otwieranie atrakcji " + std::to_string(GetAttractionID()) + "!");
    m_pauseSemaphore->SetValue(1);
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
            if (m_sharedMemory->GetData()->attractionPID[i] == 0) {
                m_sharedMemory->GetData()->attractionPID[i] = getpid();
                changed = true;
                m_attractionID = i;
                break;
            }
        }
        if (!changed)
            throw std::runtime_error("all atractions already exist!");
    });

    m_pauseSemaphore = m_semaphoreArray->GetSemaphore(
        (uint16_t) MainSemaphoreArray::AttractionPause1 + GetAttractionID());
    m_eventSemaphore = m_semaphoreArray->GetSemaphore(
        (uint16_t) MainSemaphoreArray::AttractionEvent1 + GetAttractionID());

    m_replyMQ = nullptr;
    m_attractionMQ = GetAttractionMQ(getpid(), true);
    if (!m_attractionMQ)
        throw std::runtime_error("Couldn't create attraction message queue!");

    pSetProcessRole(ProcessRole::ATTRACTION);

    pLogMessage("Atrakcja " + std::to_string(GetAttractionID()) + " rozpoczyna prace!");
}

void AttractionProc::Run() {
    while (m_sharedMemory->GetData()->isOpen) {
        pHandleAttraction();

        // attraction was closed, wait for signal to open
        if (paused)
            m_pauseSemaphore->Wait(1, true);
        paused = false;
    }
}

void AttractionProc::pHandleAttraction() {
    if (paused)
        return;

    pHandleAttractionMQ();
    sleep(1);
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
    ;
}

void AttractionProc::pHandleEnterAttraction(const AttractionMQMessage& message) {
    m_enterQueue.push_back(message);
}

void AttractionProc::pCloseImpl() {
    if (GetAttractionID() != -1)
        m_sharedMemory->GetSemLock().Execute([this]() {
            m_sharedMemory->GetData()->attractionPID[GetAttractionID()] = 0;
        });

    m_attractionMQ = nullptr;
    m_replyMQ = nullptr;

    pLogMessage("Atrakcja " + std::to_string(m_attractionID) + " konczy prace!");
}

int AttractionProc::GetAttractionID() {
    return m_attractionID;
}