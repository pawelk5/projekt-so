#pragma once
#include <sys/types.h>
#include <sys/wait.h>
#include "Process.hpp"
#include "SemaphoreArray.hpp"
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

private:
    int m_attractionID;
    Semaphore m_attractionSemaphore;

    AttractionMQ m_attractionQueue;
    ClientMQ m_replyMQ;
};