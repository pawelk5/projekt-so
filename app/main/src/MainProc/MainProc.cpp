#include "MainProc.hpp"
#include "LoggerService/LoggerService.hpp"
#include "SimulationData.hpp"
#include "IPC/Signal.hpp"
#include "Utils.hpp"
#include <csignal>
#include <cstdint>
#include <fcntl.h>
#include <iostream>
#include <pthread.h>
#include <stdexcept>
#include <sys/types.h>
#include <semaphore.h>
#include <unistd.h>

extern int g_loggerStatus;
sem_t g_loggerInitSem;
static volatile bool paused = false;

void SigintAction(int sig) {
    MainProc::Get().HandleSigint();
}

void Sigusr1Action(int sig) {
    MainProc::Get().HandleSigusr1();
}

void Sigusr2Action(int sig) {
    MainProc::Get().HandleSigusr2();
}

void MainProc::HandleSigint() {
    m_sharedMemory->GetSemLock().Execute([this]() {
        m_sharedMemory->GetData()->isOpen = false;
    });

    kill(m_sharedMemory->GetData()->cashierPID, SIGUSR1);

    pOpenAllLoopSemaphores();
}

void MainProc::HandleSigusr1() {
    m_pauseSemaphore->SetValue(0);
    paused = true;
}

void MainProc::HandleSigusr2() {
    m_pauseSemaphore->SetValue(1);
}

MainProc::MainProc() { ; }
MainProc::~MainProc() { ; }

MainProc& MainProc::Get() {
    static MainProc app;
    return app;
}

void MainProc::Run() {
    while (m_sharedMemory->GetData()->isOpen) {
        CreateProcess("park-client");
        usleep(RandomInt(CLIENT_SPAWN_TIME_MIN, CLIENT_SPAWN_TIME_MAX));
        if (paused) 
            m_pauseSemaphore->Wait();
        paused = false;
    }
}

void MainProc::pInitImpl() {
    CreateSignalHandler(SIGINT, SigintAction);
    CreateSignalHandler(SIGTERM, SigintAction);

    CreateSignalHandler(SIGUSR1, Sigusr1Action);
    CreateSignalHandler(SIGUSR2, Sigusr2Action);
    
    if (signal(SIGCHLD, SIG_IGN) == SIG_ERR)
        throw std::runtime_error("Couldn't ignore sigchld signal!");

    sem_init(&g_loggerInitSem, 0, 0);
    pthread_create(&m_loggerThread, nullptr, LoggerThread, nullptr);
    sem_wait(&g_loggerInitSem);
    sem_destroy(&g_loggerInitSem);

    if (g_loggerStatus < 1)
        throw std::runtime_error("Couldn't create logger thread!");


    m_sharedMemory->GetSemLock().Execute([this] {
        m_sharedMemory->GetData()->isOpen = true;
        m_sharedMemory->GetData()->parkSize = PARK_SIZE;
        m_sharedMemory->GetData()->mainPID = getpid();
    });
    m_pauseSemaphore = m_semaphoreArray->GetSemaphore((uint16_t) MainSemaphoreArray::MainPause);

    CreateProcess("park-cashier");
    CreateProcess("park-restaurant");

    for (int i = 0; i < ATTRACTION_COUNT - 1; i++) {
        CreateProcess("park-attraction");
    }

    pOpenAllLoopSemaphores();
}

void MainProc::pCloseImpl() {
    m_sharedMemory->GetSemLock().Execute([this]() {
        m_sharedMemory->GetData()->isOpen = false;
    });

    pOpenAllLoopSemaphores();
    while(wait(NULL) > 0) { ; }

    pLogMessage("", false, true);
    pthread_join(m_loggerThread, nullptr);
}

void MainProc::pOpenAllLoopSemaphores() {
    for (int id = (int)MainSemaphoreArray::CashierEvent; id <= (int)MainSemaphoreArray::RestaurantEvent; id++) {
        auto semaphore = m_semaphoreArray->GetSemaphore(id);
        semaphore->Signal();
    }
}