#include "CashierProc.hpp"
#include "MessageTypes/ClientMQ.hpp"
#include "PredefinedMQ.hpp"
#include <ctime>
#include <exception>
#include <iostream>
#include <memory>

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

    m_registerQueue = GetRegisterMQ(m_sharedMemory->GetData()->cashierPID, true);
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

    auto replyPID = registerMsg->senderPID;

    try {
        switch (registerMsg->mType) {
        case RegisterMessageType::ENTER_PARK:
            pHandleEnterPark(replyPID, std::get<EnterPark>(registerMsg->content));
            break;

        case RegisterMessageType::EXIT_PARK:
            break;
        
        default:
            std::cerr << "Zly typ wiadomosci!" << std::endl;
        }
    } catch (std::bad_variant_access variant_error) {
        std::cerr << "Zla zawartosc wiadomosci!\n" << variant_error.what() << std::endl;
    }
}

void CashierProc::pHandleEnterPark(pid_t replyPID, EnterPark msg) {
    ClientMQ replyMQ;
    try {
        replyMQ = GetClientMQ(replyPID);
        bool allowed = true;

        ClientMQMessage replyMsg;
        replyMsg.senderPID = getpid();
        replyMsg.mType = ClientMessageType::ENTRY_PERMIT;
        replyMsg.content = EntryPermit{ .allowed = allowed };

        replyMQ->SendMessage(replyMsg);
        // dont wait for ack message
        if (!allowed)
            return;

        auto msg = replyMQ->RecieveMessage(1);
        // no ack message
        if (!msg)
            return;

        if (msg->mType == ClientMessageType::ACK) {
            // CLIENT ENTERS PARK
            std::cout << "klient " << msg->senderPID << " wchodzi do parku!" << std::endl;
        }
    } catch (const std::exception& e) {
        replyMQ = nullptr;
        // client left the queue before response
        if (errno == EBADF) { ; }
    }    
}