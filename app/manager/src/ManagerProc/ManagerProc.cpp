#include "ManagerProc.hpp"
#include <iostream>

ManagerProc::ManagerProc() { ; }
ManagerProc::~ManagerProc() { ; }

ManagerProc& ManagerProc::Get() {
    static ManagerProc app;
    return app;
}

void ManagerProc::Run() {
    
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
    {
        auto t_semlock = m_sharedMemory->GetSemLock();
        std::cout << "manager semlock" << std::endl;
        
        m_sharedMemory->GetData()->managerPID = 0;
    }
}