#include "AttractionProc.hpp"
#include "SemaphoreLock.hpp"
#include "SimulationData.hpp"
#include <sys/types.h>

static volatile bool paused = false;

void SigUsr1(int sig) {
    std::cout << "ZAMYKANIE ATRAKCJI!" << std::endl;
    paused = true;
    AttractionProc::Get().CloseAttraction();
}

void SigUsr2(int sig) {
    std::cout << "OTWIERANIE ATRAKCJI!" << std::endl;
    paused = false;
    AttractionProc::Get().OpenAttraction();
}

void AttractionProc::CloseAttraction() {
    m_attractionSemaphore->SetValue(0);
}

void AttractionProc::OpenAttraction() {
    m_attractionSemaphore->SetValue(1);
}

AttractionProc::AttractionProc()
    :m_attractionID(-1)
{ 
    ;
}
AttractionProc::~AttractionProc() { ; }

AttractionProc& AttractionProc::Get() {
    static AttractionProc app;
    return app;
}

void AttractionProc::pInitImpl() {
    m_sharedMemory->GetSemLock().Execute([this]() {
        if (!m_sharedMemory->GetData()->isOpen)
            throw std::runtime_error("park is closed!");

        bool changed = false;
        for (int i = 0; i < ATTRACTION_COUNT; i++) {
            if (m_sharedMemory->GetData()->attractionPID[i] == 0) {
                m_sharedMemory->GetData()->attractionPID[i] = getpid();
                changed = true;
                m_attractionID = i;
                break;
            }
        }
        if (!changed)
            throw std::runtime_error("all atractions already exist!");
    });

    m_attractionSemaphore = m_semaphoreArray->GetSemaphore(
        (u_int16_t) MainSemaphoreArray::Attraction1Loop + m_attractionID);


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
}

void AttractionProc::Run() {
    while (m_sharedMemory->GetData()->isOpen) {
        pHandleAttraction();

        // attraction was closed, wait for signal to open
        if (paused)
            m_attractionSemaphore->Wait(1, true);
    }
}

void AttractionProc::pHandleAttraction() {
    if (paused)
        return;

    sleep(1);
}

void AttractionProc::pCloseImpl() {
    if (m_attractionID != -1)
        m_sharedMemory->GetSemLock().Execute([this]() {
            if (m_sharedMemory->GetData()->attractionPID[GetAttractionID()] == getpid()) 
                m_sharedMemory->GetData()->attractionPID[GetAttractionID()] = 0;
        });
}

int AttractionProc::GetAttractionID() {
    return m_attractionID;
}