#include "MainProc.hpp"
#include "SimulationData.hpp"
#include <iostream>
#include <stdexcept>

void SigintAction(int sig) {
    MainProc::Get().HandleSigint();
}

void MainProc::HandleSigint() {
    m_sharedMemory->GetSemLock().Execute([this]() {
        m_sharedMemory->GetData()->isOpen = false;
    });
}

MainProc::MainProc() { ; }
MainProc::~MainProc() { ; }

MainProc& MainProc::Get() {
    static MainProc app;
    return app;
}

void MainProc::Run() {
    while(wait(NULL) > 0) { ; }
}

void MainProc::pInitImpl() {
    struct sigaction sa;
    sa.sa_handler = SigintAction;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction SIGINT");
        throw std::runtime_error("Couldn't set up sigint handler!");
    }

    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        perror("sigaction SIGTERM");
        throw std::runtime_error("Couldn't set up sigterm handler!");
    }


    m_sharedMemory->GetSemLock().Execute([this] {
        m_sharedMemory->GetData()->isOpen = true;
        m_sharedMemory->GetData()->parkSize = PARK_SIZE;
    });

    if (fork() == 0)
        execl("./park-manager", "park-manager", NULL);

    if (fork() == 0)
        execl("./park-cashier", "park-cashier", NULL);

    if (fork() == 0)
        execl("./park-restaurant", "park-restaurant", NULL);


    for (int i = 0; i < ATTRACTION_COUNT - 1; i++){
        if (fork() == 0)
            execl("./park-attraction", "park-attraction", NULL);
    }

    sleep(1);
    for (int i = 0; i < 30; i++) {
        if (fork() == 0)
            execl("./park-client", "park-client", NULL);
    }
}

void MainProc::pCloseImpl() {
    m_sharedMemory->GetSemLock().Execute([this]() {
        m_sharedMemory->GetData()->isOpen = false;
    });

    while(wait(NULL) > 0) { ; }

    m_sharedMemory->GetSemLock().Execute([this]() {
        std::cout << m_sharedMemory->GetData()->managerPID << std::endl;
        std::cout << m_sharedMemory->GetData()->cashierPID << std::endl;

        std::cout << "Attractions: " << std::endl;
        for (int i = 0; i < ATTRACTION_COUNT; i++) 
            std::cout << i << ": " << m_sharedMemory->GetData()->attractionPID[i] << std::endl;
    });
}