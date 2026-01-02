#include "CashierProc.hpp"
#include "PredefinedMQ.hpp"
#include <iostream>

CashierProc::CashierProc() { ; }
CashierProc::~CashierProc() { ; }

CashierProc& CashierProc::Get() {
    static CashierProc app;
    return app;
}

void CashierProc::Run() {
    while (m_sharedMemory->GetData()->isOpen) {
        pHandleRegisterMQ();
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

    m_registerQueue = GetCashierMQ(m_sharedMemory->GetData()->cashierPID, true);
}

void CashierProc::pCloseImpl() {
    m_sharedMemory->GetSemLock().Execute([this]() {     
        if (m_sharedMemory->GetData()->cashierPID == getpid()) 
            m_sharedMemory->GetData()->cashierPID = 0;
    });
    
    m_registerQueue = nullptr;
}

void CashierProc::pHandleRegisterMQ() {
    auto registerMsg = m_registerQueue->RecieveMessage(1);
    if (!registerMsg)
        return;

    std::cout << "Kasa otrzymala wiadomosc od " << registerMsg->senderPID << std::endl;
    std::cout << "Typ wiadomosci: " << (int)registerMsg->mType << std::endl;
}