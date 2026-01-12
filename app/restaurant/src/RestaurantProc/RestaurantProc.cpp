#include "RestaurantProc.hpp"
#include "SimulationData.hpp"

static volatile bool paused = false;

void SigUsr1(int sig) {
    paused = true;
    RestaurantProc::Get().CloseAttraction();
}

void SigUsr2(int sig) {
    paused = false;
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
            m_restaurantSemaphore->Wait(1, true);
    }
}

void RestaurantProc::pInitImpl() {
    struct sigaction sa;
    sa.sa_handler = SigUsr1;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("sigaction SIGUSR1");
        throw std::runtime_error("Couldn't set up SIGUSR1 handler!");
    }

    sa.sa_handler = SigUsr2;

    if (sigaction(SIGUSR2, &sa, NULL) == -1) {
        perror("sigaction SIGUSR2");
        throw std::runtime_error("Couldn't set up SIGUSR2 handler!");
    }

    m_sharedMemory->GetSemLock().Execute([this]() {
        if (!m_sharedMemory->GetData()->isOpen)
            throw std::runtime_error("park is closed!");
        
        if (m_sharedMemory->GetData()->attractionPID[RESTAURANT_INDEX] != 0)
            throw std::runtime_error("restaurant already exists!");

        m_sharedMemory->GetData()->attractionPID[RESTAURANT_INDEX] = getpid();
    });

    m_restaurantSemaphore = m_semaphoreArray->GetSemaphore((uint16_t)MainSemaphoreArray::RestaurantLoop);
}

void RestaurantProc::pCloseImpl() {
    m_sharedMemory->GetSemLock().Execute([this]() {    
        if (m_sharedMemory->GetData()->attractionPID[RESTAURANT_INDEX] == getpid())  
            m_sharedMemory->GetData()->attractionPID[RESTAURANT_INDEX] = 0;
    });
    m_restaurantSemaphore = nullptr;
}

void RestaurantProc::CloseAttraction() {
    pLogMessage("Zamykanie restauracji!");
    m_restaurantSemaphore->SetValue(0);
}

void RestaurantProc::OpenAttraction() {
    pLogMessage("Otwieranie restauracji!");
    m_restaurantSemaphore->SetValue(1);
}