#include "CashierProc.hpp"
#include "MessageTypes/ClientMQ.hpp"
#include "PredefinedMQ.hpp"
#include "SimulationData.hpp"
#include <cerrno>
#include <ctime>
#include <exception>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

CashierProc::CashierProc() { ; }
CashierProc::~CashierProc() { ; }

CashierProc& CashierProc::Get() {
    static CashierProc app;
    return app;
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
    m_eventSemaphore = m_semaphoreArray->GetSemaphore((uint16_t) MainSemaphoreArray::CashierEvent);
    m_clientCounter = 0;
    pSetProcessRole(ProcessRole::CASHIER);

    pLogMessage("Kasa rozpoczyna prace!");
}

void CashierProc::Run() {
    while (m_sharedMemory->GetData()->isOpen || m_clients.size() > 0) {
        m_eventSemaphore->Wait(1, true);
        pHandleRegisterMQ();

        while (m_enterVipQueue.size() > 0) {
            if (!pRegisterClient(m_enterVipQueue[0]))
                break;
            m_enterVipQueue.erase(m_enterVipQueue.begin());
        }

        while (m_enterVipQueue.size() == 0 && m_enterQueue.size() > 0) {
            if (!pRegisterClient(m_enterQueue[0]))
                break;
            m_enterQueue.erase(m_enterQueue.begin());
        }

        pLogMessage("Aktualna liczba klientow: " + std::to_string(m_clientCounter));
    }
}

void CashierProc::pCloseImpl() {
    m_registerQueue = nullptr;
    m_sharedMemory->GetSemLock().Execute([this]() {     
        if (m_sharedMemory->GetData()->cashierPID == getpid()) 
            m_sharedMemory->GetData()->cashierPID = 0;
    });
    
    m_replyMQ = nullptr;
    pLogMessage("Kasa konczy prace!");
}

void CashierProc::pHandleRegisterMQ() {
    auto registerMsg = m_registerQueue->ReceiveMessage(true, DEFAULT_MQ_TIMEOUT);
    if (!registerMsg)
        return;

    auto replyPID = registerMsg->senderPID;

    try {
        switch (registerMsg->mType) {
        case RegisterMessageType::ENTER_PARK:
            pHandleEnterPark(*registerMsg);
            break;

        case RegisterMessageType::EXIT_PARK:
            pHandleExitPark(*registerMsg);
            pRemoveClient(replyPID);
            break;
        
        default:
            std::cerr << "Zly typ wiadomosci!" << std::endl;
        }
    } catch (std::bad_variant_access variant_error) {
        std::cerr << "Zla zawartosc wiadomosci!\n" << variant_error.what() << std::endl;
    }
}

void CashierProc::pHandleEnterPark(const RegisterMQMessage& message) {
    auto msg = std::get<EnterPark>(message.content);
    if (msg.isVip)
        m_enterVipQueue.push_back(message);
    else
        m_enterQueue.push_back(message);
}

void CashierProc::pHandleExitPark(const RegisterMQMessage& message) {
    auto replyPID = message.senderPID;
    if (!m_clients.contains(replyPID)) {
        pLogMessage("Blad: klient (" + std::to_string(replyPID) + ") spoza parku wyslal wiadomosc o wyjsciu z parku!");
        return;
    }

    if (m_clients.at(replyPID).vip) {
        pLogMessage("Vip o pid " + std::to_string(replyPID) + " opuszcza park!");
        return;
    }

    try {
        auto msgContent = std::get<ExitPark>(message.content);

        ClientMQMessage replyMsg;
        replyMsg.senderPID = getpid();
        replyMsg.mType = ClientMessageType::BILL;
        replyMsg.content = Bill{ .price = pCalculatePrice(replyPID, msgContent.visitedRestaurant) };
        
        if (!pCreateReplyMQ(replyPID))
            return;

        if (!pSendReply(replyPID, replyMsg))
            throw std::runtime_error("Nie mozna bylo wyslac odpowiedzi do klienta!");

        auto msg = m_replyMQ->ReceiveMessage(true, DEFAULT_MQ_TIMEOUT);
        // no ack message
        if (!msg)
            throw std::runtime_error("Klient nie wyslal potwierdzenia rachunku!");

        if (msg->mType == ClientMessageType::ACK) {
            // CLIENT LEAVES PARK
            pLogMessage("Klient " + std::to_string(replyPID) + " wychodzi z parku!");
        }
    } catch (const std::exception& e) {
        // client left the queue before response
        pLogMessage("Przy obsludze klienta " + std::to_string(replyPID) + " nastapil blad w komunikacji!");
        pLogMessage((std::string)"BLAD: " + e.what());
    }

    m_replyMQ = nullptr;
}

// returns false only if no more clients can be registered
bool CashierProc::pRegisterClient(const RegisterMQMessage& message) {
    auto replyPID = message.senderPID;
    try {
        auto msgContent = std::get<EnterPark>(message.content);
        bool allowed = m_sharedMemory->GetData()->isOpen;

        if ((m_sharedMemory->GetData()->parkSize < m_clientCounter + (msgContent.hasChild + 1))
            && allowed)
            return false;

        ClientMQMessage replyMsg;
        replyMsg.senderPID = getpid();
        replyMsg.mType = ClientMessageType::PARK_ENTRY_PERMIT;
        
        replyMsg.content = ParkEntryPermit{ .allowed = allowed };

        if (!pCreateReplyMQ(replyPID))
            return true;

        if (!pSendReply(replyPID, replyMsg))
            return true;

        // dont wait for ack message
        if (!allowed)
            return true;

        auto msg = m_replyMQ->ReceiveMessage(true, DEFAULT_MQ_TIMEOUT);
        // no ack message
        if (!msg)
            return true;

        if (msg->mType == ClientMessageType::ACK) {
            // CLIENT ENTERS PARK
            pLogMessage("klient " + std::to_string(replyPID) + " wchodzi do parku!");

            ClientData data {
                .hasChild = msgContent.hasChild,
                .vip = msgContent.isVip,

                .entryTime = time(NULL),
                .ticketType = msgContent.ticketType
            };
            m_clientCounter += msgContent.hasChild + 1;
            m_clients[message.senderPID] = data;
        }
    } catch (const std::exception& e) {
        // client left the queue before response
        pLogMessage("Przy obsludze klienta " + std::to_string(replyPID) + " nastapil blad w komunikacji!");
        pLogMessage((std::string)"BLAD: " + e.what());
    }

    m_replyMQ = nullptr;
    return true;
}

float CashierProc::pCalculatePrice(pid_t pid, bool usedRestaurant) {
    const auto ct_clientData = m_clients.at(pid);
    const auto ct_ticketData = TicketConfig.at(m_clients.at(pid).ticketType);
    float price = ct_ticketData.price;

    if (usedRestaurant)
        price *= 1.1f;

    // overtime
    int overtime = (ct_clientData.entryTime + ct_ticketData.time) - time(NULL);

    if (overtime > 0) {
        float mfactor = 1.0f + (float)overtime / ct_ticketData.time;
        price *= mfactor;
    }

    // ticket for child
    if (ct_clientData.hasChild)
        price *= 1.5f;

    return price;
}

void CashierProc::pRemoveClient(pid_t pid) {
    if (!m_clients.contains(pid))
        return;
    
    m_clientCounter -= (1 + m_clients.at(pid).hasChild);
    m_clients.erase(pid);
}

bool CashierProc::pCreateReplyMQ(pid_t pid) {
    m_replyMQ = GetClientParkMQ(pid, false, [this, pid] {
        if (errno == ENOENT) {
            pLogMessage("Klient " + std::to_string(pid) + " przestal dzialac przed odebraniem wiadomosci z kasy!");
            return true;
        }
        return false;
    });

    return m_replyMQ != nullptr;
}

bool CashierProc::pSendReply(pid_t pid, const ClientMQMessage& msg) {
    return m_replyMQ->SendMessage(msg,
        [this, pid] {
            if (errno == EBADF) {
                pLogMessage("Klient " + std::to_string(pid) + " opuscil kolejke przed odebraniem wiadomosci!");
                return true;
            }
            return false;
        }, true, 0, DEFAULT_MQ_TIMEOUT);
}