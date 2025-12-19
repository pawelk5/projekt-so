#include "CashierProc.hpp"
#include <iostream>

CashierProc::CashierProc() { ; }
CashierProc::~CashierProc() { ; }

CashierProc& CashierProc::Get() {
    static CashierProc app;
    return app;
}

void CashierProc::Run() {
    
}

void CashierProc::pInitImpl() {
    m_sharedMemory->WithSemLock([this]() {
        if (m_sharedMemory->GetData()->cashierPID != 0)
            throw std::runtime_error("cashier already exists!");

        m_sharedMemory->GetData()->cashierPID = getpid();
    });
}

void CashierProc::pCloseImpl() {
    m_sharedMemory->WithSemLock([this]() {      
        m_sharedMemory->GetData()->cashierPID = 0;
    });
}