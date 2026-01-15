#pragma once
#include <sys/types.h>
#include <sys/wait.h>
#include "Process.hpp"
#include "IPC/SemaphoreArray.hpp"
#include "MessageTypes/RestaurantMQ.hpp"
#include "MessageTypes/ClientMQ.hpp"

class RestaurantProc : public Process {
public:
    static RestaurantProc& Get();
    void Run();

    ~RestaurantProc();
    void CloseAttraction();
    void OpenAttraction();
protected:
    RestaurantProc();
    void pInitImpl() override;
    void pCloseImpl() override;

private:
    Semaphore m_pauseSemaphore;
    Semaphore m_eventSemaphore;

    RestaurantMQ m_restaurationMQ;
    ClientMQ m_replyMQ;
};