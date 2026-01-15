#include "MainProc.hpp"
#include "LoggerService/LoggerService.hpp"
#include "SimulationData.hpp"
#include "IPC/Signal.hpp"
#include "Utils.hpp"
#include <fcntl.h>
#include <iostream>
#include <pthread.h>
#include <stdexcept>
#include <sys/types.h>
#include <semaphore.h>
#include <unistd.h>

extern int g_loggerStatus;
sem_t g_loggerInitSem;

void SigintAction(int sig) {
    MainProc::Get().HandleSigint();
}

void MainProc::HandleSigint() {
    m_sharedMemory->GetSemLock().Execute([this]() {
        m_sharedMemory->GetData()->isOpen = false;
    });

    pOpenAllLoopSemaphores();
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
    }
}

void MainProc::pInitImpl() {
    CreateSignalHandler(SIGINT, SigintAction);
    CreateSignalHandler(SIGTERM, SigintAction);
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
    for (int id = (int)MainSemaphoreArray::CashierLoop; id <= (int)MainSemaphoreArray::RestaurantLoop; id++) {
        auto semaphore = m_semaphoreArray->GetSemaphore(id);
        semaphore->Signal();
    }
}