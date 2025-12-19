#pragma once
#include <sys/types.h>
#include <sys/wait.h>
#include "Process.hpp"

class CashierProc : public Process {
public:
    static CashierProc& Get();
    void Run();

    ~CashierProc();

protected:
    CashierProc();
    void pInitImpl() override;
    void pCloseImpl() override;

};