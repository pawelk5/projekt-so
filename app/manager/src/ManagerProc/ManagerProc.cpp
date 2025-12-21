#include "ManagerProc.hpp"
#include <iostream>
#include <unistd.h>

ManagerProc::ManagerProc() { ; }
ManagerProc::~ManagerProc() { ; }

ManagerProc& ManagerProc::Get() {
    static ManagerProc app;
    return app;
}

void ManagerProc::Run() {
    sleep(2);
}

void ManagerProc::pInitImpl() {
    {
        m_sharedMemory->WithSemLock([this]() {
            if (m_sharedMemory->GetData()->managerPID != 0)
                throw std::runtime_error("manager already exists!");

            m_sharedMemory->GetData()->managerPID = getpid();
        });
    }
}

void ManagerProc::pCloseImpl() {
    sleep(1);
    m_sharedMemory->WithSemLock([this]() {
        if (m_sharedMemory->GetData()->managerPID == getpid())
            m_sharedMemory->GetData()->managerPID = 0;
    });
}