#pragma once
#include <sys/types.h>
#include <sys/wait.h>
#include "Process.hpp"

class ClientProc : public Process {
public:
    static ClientProc& Get();
    void Run();

    ~ClientProc();

protected:
    ClientProc();
    void pInitImpl() override;
    void pCloseImpl() override;

};