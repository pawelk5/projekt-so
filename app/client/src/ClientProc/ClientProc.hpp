#pragma once
#include <sys/types.h>
#include <sys/wait.h>
#include "Process.hpp"
#include "MessageTypes/ClientMQ.hpp"
class ClientProc : public Process {
public:
    static ClientProc& Get();
    void Run();

    ~ClientProc();

protected:
    ClientProc();
    void pInitImpl() override;
    void pCloseImpl() override;

private:
    ClientMQ m_clientQueue;
};