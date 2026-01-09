#include "CashierProc.hpp"
#include "MessageTypes/ClientMQ.hpp"
#include "PredefinedMQ.hpp"
#include <cerrno>
#include <ctime>
#include <exception>
#include <iostream>
#include <memory>
#include <string>

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
    auto registerMsg = m_registerQueue->RecieveMessage(true, 1);
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
        bool allowed = true;

        ClientMQMessage replyMsg;
        replyMsg.senderPID = getpid();
        replyMsg.mType = ClientMessageType::ENTRY_PERMIT;
        replyMsg.content = EntryPermit{ .allowed = allowed };

        replyMQ = GetClientMQ(replyPID, false, [this, replyPID] {
            if (errno == ENOENT) {
                pLogMessage("Klient " + std::to_string(replyPID) + " opuscil kolejke przed odebraniem wiadomosci!");
                return true;
            }
            return false;
        });
        

        bool result = replyMQ->SendMessage(replyMsg,
            [this, replyPID] {
                if (errno == EBADF) {
                    pLogMessage("Klient " + std::to_string(replyPID) + " opuscil kolejke przed odebraniem wiadomosci!");
                    return true;
                }
                return false;
            },
            true, 0, 1);

        if (!result)
            return;
        // dont wait for ack message
        if (!allowed)
            return;

        auto msg = replyMQ->RecieveMessage(true, 1);
        // no ack message
        if (!msg)
            return;

        if (msg->mType == ClientMessageType::ACK) {
            // CLIENT ENTERS PARK
            pLogMessage("klient " + std::to_string(replyPID) + " wchodzi do parku!");
        }
    } catch (const std::exception& e) {
        replyMQ = nullptr;
        // client left the queue before response
        pLogMessage("Przy obsludze klienta " + std::to_string(replyPID) + " nastapil blad w komunikacji!");
    }
}