#pragma once
#include <sys/types.h>
#include <sys/wait.h>
#include "Process.hpp"

class MainProc : public Process {
public:
    static MainProc& Get();
    void Run();

protected:
    void pInitImpl() override;
    void pCloseImpl() override;

private:
    MainProc();
    ~MainProc();

};