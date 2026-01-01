#include "CashierProc.hpp"
#include <iostream>

CashierProc::CashierProc() { ; }
CashierProc::~CashierProc() { ; }

CashierProc& CashierProc::Get() {
    static CashierProc app;
    return app;
}

void CashierProc::Run() {
    while (m_sharedMemory->GetData()->isOpen) {
        sleep(1);
    }
}

void CashierProc::pInitImpl() {
    m_sharedMemory->GetSemLock().Execute([this]() {
        if (!m_sharedMemory->GetData()->isOpen)
            throw std::runtime_error("park is closed!");

        if (m_sharedMemory->GetData()->cashierPID != 0)
            throw std::runtime_error("cashier already exists!");

        m_sharedMemory->GetData()->cashierPID = getpid();
    });
}

void CashierProc::pCloseImpl() {
    m_sharedMemory->GetSemLock().Execute([this]() {     
        if (m_sharedMemory->GetData()->cashierPID == getpid()) 
            m_sharedMemory->GetData()->cashierPID = 0;
    });
}