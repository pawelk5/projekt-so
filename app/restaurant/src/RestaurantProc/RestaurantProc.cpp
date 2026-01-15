#include "RestaurantProc.hpp"
#include "PredefinedMQ.hpp"
#include "SimulationData.hpp"
#include "IPC/Signal.hpp"

static volatile bool paused = false;

void SigUsr1(int sig) {
    paused = true;
    RestaurantProc::Get().CloseAttraction();
}

void SigUsr2(int sig) {
    RestaurantProc::Get().OpenAttraction();
}

RestaurantProc::RestaurantProc() { ; }
RestaurantProc::~RestaurantProc() { ; }

RestaurantProc& RestaurantProc::Get() {
    static RestaurantProc app;
    return app;
}

void RestaurantProc::Run() {
    while (m_sharedMemory->GetData()->isOpen) {
        sleep(1);

        // attraction was closed, wait for signal to open
        if (paused)
            m_pauseSemaphore->Wait(1, true);
        paused = false;
    }
}

void RestaurantProc::pInitImpl() {
    CreateSignalHandler(SIGUSR1, SigUsr1);
    CreateSignalHandler(SIGUSR2, SigUsr2);

    m_sharedMemory->GetSemLock().Execute([this]() {
        if (!m_sharedMemory->GetData()->isOpen)
            throw std::runtime_error("park is closed!");
        
        if (m_sharedMemory->GetData()->attractionPID[RESTAURANT_INDEX] != 0)
            throw std::runtime_error("restaurant already exists!");

        m_sharedMemory->GetData()->attractionPID[RESTAURANT_INDEX] = getpid();
    });

    m_pauseSemaphore = m_semaphoreArray->GetSemaphore((uint16_t)MainSemaphoreArray::RestaurantPause);
    m_loopSemaphore = m_semaphoreArray->GetSemaphore((uint16_t)MainSemaphoreArray::RestaurantEvent);

    m_replyMQ = nullptr;
    m_restaurationMQ = GetRestaurantMQ(getpid(), true);
    if (!m_restaurationMQ)
        throw std::runtime_error("Couldn't create attraction message queue!");

    pSetProcessRole(ProcessRole::RESTAURANT);
    pLogMessage("Restauracja rozpoczyna prace!");
}

void RestaurantProc::pCloseImpl() {
    m_sharedMemory->GetSemLock().Execute([this]() {    
        if (m_sharedMemory->GetData()->attractionPID[RESTAURANT_INDEX] == getpid())  
            m_sharedMemory->GetData()->attractionPID[RESTAURANT_INDEX] = 0;
    });

    m_restaurationMQ = nullptr;
    m_replyMQ = nullptr;
    m_pauseSemaphore = nullptr;

    pLogMessage("Restauracja konczy prace!");
}

void RestaurantProc::CloseAttraction() {
    pLogMessage("Zamykanie restauracji!");
    m_pauseSemaphore->SetValue(0);
}

void RestaurantProc::OpenAttraction() {
    pLogMessage("Otwieranie restauracji!");
    m_pauseSemaphore->SetValue(1);
}