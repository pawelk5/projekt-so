#pragma once
#include <sys/types.h>
#include <sys/wait.h>
#include "IPC/SemaphoreArray.hpp"
#include "Process.hpp"

class MainProc : public Process {
public:
    static MainProc& Get();
    void Run();

    void HandleSigint();
    void HandleSigusr1();
    void HandleSigusr2();

protected:
    void pInitImpl() override;
    void pCloseImpl() override;

private:
    MainProc();
    ~MainProc();

private:
    void pOpenAllLoopSemaphores();

private:
    pthread_t m_loggerThread;
    Semaphore m_pauseSemaphore;
};