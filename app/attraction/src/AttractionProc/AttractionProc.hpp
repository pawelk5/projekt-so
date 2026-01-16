#pragma once
#include <sys/types.h>
#include <sys/wait.h>
#include <vector>
#include <map>
#include "AttractionHandler.hpp"
#include "Process.hpp"
#include "MessageTypes/AttractionMQ.hpp"
#include "MessageTypes/ClientMQ.hpp"
#include "SimulationData.hpp"


class AttractionProc : public Process {
public:
    static AttractionProc& Get();
    void Run();

    ~AttractionProc();
    
    int GetAttractionID();
    
    void CloseAttraction();
    void OpenAttraction();
protected:
    AttractionProc();
    void pInitImpl() override;
    void pCloseImpl() override;

private:
    void pHandleAttraction();
    void pHandleAttractionMQ();

    void pHandleEnterAttraction(const AttractionMQMessage& message);
    void pRemoveClient(pid_t pid);

private:
    bool pCreateReplyMQ(pid_t pid);
    bool pCreateNewHandler();
    bool pRegisterClient(const AttractionMQMessage& message, std::shared_ptr<AttractionHandler> handler);
    bool pSendReply(pid_t pid, const ClientMQMessage& msg);
    time_t pGetNextTimeout();

private:
    int m_attractionID;
    Semaphore m_pauseSemaphore;
    Semaphore m_eventSemaphore;

    AttractionMQ m_attractionMQ;
    ClientMQ m_replyMQ;

private:
    __AttractionConfig cm_attractionConfig;

private:
    std::vector<AttractionMQMessage> m_enterQueue;

    /// SEM ID : attraction handler map
    std::map<uint16_t, std::shared_ptr<AttractionHandler>> m_attractionHandlers;
};