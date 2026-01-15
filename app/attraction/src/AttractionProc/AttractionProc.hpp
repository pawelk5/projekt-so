#pragma once
#include <sys/types.h>
#include <sys/wait.h>
#include "Process.hpp"
#include "MessageTypes/AttractionMQ.hpp"
#include "MessageTypes/ClientMQ.hpp"


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
    void pHandleRegisterMQ();

private:
    int m_attractionID;
    Semaphore m_pauseSemaphore;

    AttractionMQ m_attractionMQ;
    ClientMQ m_replyMQ;

private:
    std::vector<AttractionMQMessage> m_enterQueue;
};