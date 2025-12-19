#pragma once
#include <sys/types.h>
#include <sys/wait.h>
#include "Process.hpp"

class ManagerProc : public Process {
public:
    static ManagerProc& Get();
    void Run();

    ~ManagerProc();

protected:
    ManagerProc();
    void pInitImpl() override;
    void pCloseImpl() override;

};