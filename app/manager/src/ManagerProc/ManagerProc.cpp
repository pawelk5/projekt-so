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
    while (m_sharedMemory->GetData()->isOpen) {
        sleep(1);
    }
}

void ManagerProc::pInitImpl() {
    m_sharedMemory->GetSemLock().Execute([this]() {
        if (!m_sharedMemory->GetData()->isOpen)
            throw std::runtime_error("park is closed!");
        
        if (m_sharedMemory->GetData()->managerPID != 0)
            throw std::runtime_error("manager already exists!");
        m_sharedMemory->GetData()->managerPID = getpid();
    });
}

void ManagerProc::pCloseImpl() {
    sleep(1);
    m_sharedMemory->GetSemLock().Execute([this]() {
        if (m_sharedMemory->GetData()->managerPID == getpid())
            m_sharedMemory->GetData()->managerPID = 0;
    });
}