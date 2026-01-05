#pragma once
#include <sys/types.h>
#include <sys/wait.h>
#include "MessageTypes/RegisterMQ.hpp"
#include "Process.hpp"
#include "PredefinedMQ.hpp"

class CashierProc : public Process {
public:
    static CashierProc& Get();
    void Run();

    ~CashierProc();

protected:
    CashierProc();
    void pInitImpl() override;
    void pCloseImpl() override;

private:
    void pHandleRegisterMQ();

private:
    RegisterMQ m_registerQueue;
};