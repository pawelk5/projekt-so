#include "RestaurantProc.hpp"
#include "Config/Config.hpp"
#include "MessageTypes/RestaurantMQ.hpp"
#include "PredefinedMQ.hpp"
#include "SimulationData.hpp"
#include "IPC/Signal.hpp"
#include <cstdint>
#include <exception>
#include <string>

static volatile bool paused = false;
static volatile bool evac = false;

void SigUsr1(int sig) {
    paused = true;
    RestaurantProc::Get().CloseAttraction();
}

void SigUsr2(int sig) {
    paused = false;
    RestaurantProc::Get().OpenAttraction();
}

void SigInt(int sig) {
    evac = true;
    RestaurantProc::Get().HandleSigint();
}

RestaurantProc::RestaurantProc() { ; }
RestaurantProc::~RestaurantProc() { ; }

RestaurantProc& RestaurantProc::Get() {
    static RestaurantProc app;
    return app;
}

void RestaurantProc::Run() {
    while (m_sharedMemory->GetData()->isOpen || (m_handler && !m_handler->IsEmpty())) {
        auto timeout = pGetNextTimeout();

        m_eventSemaphore->Wait(1, false, false, timeout);
        if (!evac && !paused && m_handler) {
            if (m_handler->Finished())
                m_handler = nullptr;
        }

        pHandleRestaurantMQ();
        if (!m_handler && !paused && !evac)
            pCreateAttractionHandler();

    }
}

void RestaurantProc::pInitImpl() {
    CreateSignalHandler(SIGUSR1, SigUsr1);
    CreateSignalHandler(SIGUSR2, SigUsr2);
    CreateSignalHandler(SIGINT, SigInt);

    m_sharedMemory->GetSemLock().Execute([this]() {
        if (!m_sharedMemory->GetData()->isOpen)
            throw std::runtime_error("park is closed!");
        
        if (m_sharedMemory->GetData()->attractionPID.at(RESTAURANT_INDEX) != 0)
            throw std::runtime_error("restaurant already exists!");

        m_sharedMemory->GetData()->attractionPID.at(RESTAURANT_INDEX) = getpid();
    });

    m_eventSemaphore = m_semaphoreArray->GetSemaphore((uint16_t)MainSemaphoreArray::RestaurantEvent);
    m_handlerSemaphore = m_semaphoreArray->GetSemaphore((uint16_t)MainSemaphoreArray::RestaurantHandler);

    cm_attractionConfig = AttractionConfig.at(RESTAURANT_INDEX);
    m_replyMQ = nullptr;
    m_handler = nullptr;
    m_restaurantMQ = GetRestaurantMQ(getpid(), true);

    if (!m_restaurantMQ)
        throw std::runtime_error("Couldn't create attraction message queue!");

    pSetProcessRole(ProcessRole::RESTAURANT);
    pLogMessage("Restauracja rozpoczyna prace!");
}

void RestaurantProc::pHandleRestaurantMQ() {
    auto clientMsg = m_restaurantMQ->ReceiveMessage(true, DEFAULT_MQ_TIMEOUT);
    if (!clientMsg)
        return;

    auto replyPID = clientMsg->senderPID;

    try {
        switch (clientMsg->mType) {
        case RestaurantMessageType::ENTER_RESTAURANT:
            pHandleEnterRestaurant(*clientMsg);
            break;

        case RestaurantMessageType::EXIT_RESTAURANT:
            try {
                pSendBill(replyPID);
            } catch (std::exception e) {
                pRemoveClient(replyPID);
                throw;
            }
            pRemoveClient(replyPID);
            break;
        default:
            std::cerr << "Zly typ wiadomosci!" << std::endl;
        }
    } catch (std::exception e) {
        std::cerr << "BLAD komunikacji z klientem (restauracja)!\n" << e.what() << std::endl;
    }
}

void RestaurantProc::pCreateAttractionHandler() {
    AttractionHandlerData newHandlerData {
        .leaveSemaphore = m_semaphoreArray->GetSemaphore((uint16_t)MainSemaphoreArray::RestaurantHandler),
        .maxCientCount = cm_attractionConfig.maxClientsPerHandler,
        .attractionDuration = cm_attractionConfig.duration,
        .isRestaurant = true
    };

    auto newHandler = std::make_shared<AttractionHandler>(newHandlerData);

    while (m_enterQueue.size() > 0) {
        if (!pRegisterClient(m_enterQueue[0], newHandler))
            break;
        m_enterQueue.erase(m_enterQueue.begin());
    }

    if (!newHandler->IsEmpty()) {
        m_handler = newHandler;
        m_handler->StartAttraction();
        pLogMessage("Restauracja rozpoczyna prace z " 
             + std::to_string(m_handler->GetClientCount()) + "/" +
              std::to_string(cm_attractionConfig.maxClientsPerHandler) + " klientami!");
    }
}

void RestaurantProc::pCloseImpl() {
    m_sharedMemory->GetSemLock().Execute([this]() {    
        if (m_sharedMemory->GetData()->attractionPID.at(RESTAURANT_INDEX) == getpid())  
            m_sharedMemory->GetData()->attractionPID.at(RESTAURANT_INDEX) = 0;
    });

    m_restaurantMQ = nullptr;
    m_replyMQ = nullptr;

    pLogMessage("Restauracja konczy prace!");
}

void RestaurantProc::CloseAttraction() {
    pLogMessage("Zamykanie restauracji!");
    if (m_handler)
        m_handler->ReleaseClients();
}

void RestaurantProc::OpenAttraction() {
    pLogMessage("Otwieranie restauracji!");
}

void RestaurantProc::HandleSigint() {
    if (m_handler == nullptr)
        return;

    const auto& clientList = m_handler->GetClientList();
    for (const auto& client : clientList) {
        if (!client.second.fromPark)
            kill(client.first, SIGUSR1);
    }
}


bool RestaurantProc::pRegisterClient(const RestaurantMQMessage& message, std::shared_ptr<AttractionHandler> handler) {
    auto replyPID = message.senderPID;
    try {
        auto msgContent = std::get<EnterRestaurant>(message.content);
        bool allowed = m_sharedMemory->GetData()->isOpen;

        if ((handler->GetClientCount() + 1 + msgContent.hasChild > cm_attractionConfig.maxClientsPerHandler)
            && allowed)
            return false;

        ClientMQMessage replyMsg;
        replyMsg.senderPID = getpid();
        replyMsg.mType = ClientMessageType::ATTRACTION_ENTRY_PERMIT;

        replyMsg.content = AttractionEntryPermit{ .allowed = allowed, .leaveSemaphoreID = (uint16_t)handler->GetHandlerData().leaveSemaphore->GetSemaphoreID() };

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
            // TODO
            pLogMessage("Klient " + std::to_string(replyPID) + " wchodzi do restauracji!");
            handler->AddClient(replyPID, msgContent.hasChild, msgContent.fromPark);
        }
    } catch (const std::exception& e) {
        // client left the queue before response
        pLogMessage("Przy obsludze klienta " + std::to_string(replyPID) + " nastapil blad w komunikacji (restauracja)!");
        pLogMessage((std::string)"BLAD: " + e.what());
    }

    m_replyMQ = nullptr;
    return true;
}

bool RestaurantProc::pSendReply(pid_t pid, const ClientMQMessage& msg) {
    return m_replyMQ->SendMessage(msg,
        [this, pid] {
            if (errno == EBADF) {
                pLogMessage("Klient " + std::to_string(pid) + " opuscil kolejke przed odebraniem wiadomosci (restauracja)!");
                return true;
            }
            return false;
        }, true, 0, DEFAULT_MQ_TIMEOUT);
}

bool RestaurantProc::pCreateReplyMQ(pid_t pid) {
    m_replyMQ = GetClientRestaurantMQ(pid, false, [this, pid] {
        if (errno == ENOENT)
            return true;
        return false;
    });

    return m_replyMQ != nullptr;
}

void RestaurantProc::pHandleEnterRestaurant(const RestaurantMQMessage& message) {
    m_enterQueue.push_back(message);
}

void RestaurantProc::pRemoveClient(pid_t pid) {
    if (m_handler)
        m_handler->RemoveClient(pid);
}

void RestaurantProc::pSendBill(pid_t pid) {
    if (!m_handler)
        return;

    if (!m_handler->GetClientList().contains(pid))
        return;

    if (m_handler->GetClientList().at(pid).fromPark)
        return;

    const auto& clientData = m_handler->GetClientList().at(pid);

    try {
        ClientMQMessage replyMsg;
        replyMsg.senderPID = getpid();
        replyMsg.mType = ClientMessageType::BILL;
        replyMsg.content = Bill{ .price = 40.f + (20.f * clientData.hasChild) };
        
        if (!pCreateReplyMQ(pid))
            return;

        if (!pSendReply(pid, replyMsg))
            throw std::runtime_error("Nie mozna bylo wyslac odpowiedzi do klienta (restauracja)!");

        auto msg = m_replyMQ->ReceiveMessage(true, DEFAULT_MQ_TIMEOUT);
        // no ack message
        if (!msg)
            throw std::runtime_error("Klient nie wyslal potwierdzenia rachunku (restauracja)!");

        if (msg->mType == ClientMessageType::ACK) {
            // CLIENT LEAVES PARK
            pLogMessage("Klient " + std::to_string(pid) + " wychodzi z restauracji z rachunkiem!");
        }
    } catch (const std::exception& e) {
        // client left the queue before response
        pLogMessage("Przy obsludze klienta " + std::to_string(pid) + " nastapil blad w komunikacji!");
        pLogMessage((std::string)"BLAD: " + e.what());
    }

    m_replyMQ = nullptr;
}

time_t RestaurantProc::pGetNextTimeout() {
    time_t nextTimeout = -1;
    if (m_handler)
        nextTimeout = m_handler->GetAttractionFinishTime() - time(NULL);

    return nextTimeout;
}