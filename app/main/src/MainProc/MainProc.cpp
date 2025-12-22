#include "MainProc.hpp"
#include "SimulationData.hpp"
#include <iostream>

MainProc::MainProc() { ; }
MainProc::~MainProc() { ; }

MainProc& MainProc::Get() {
    static MainProc app;
    return app;
}

void MainProc::Run() {
    sleep(1);
    m_sharedMemory->GetSemLock().Execute([this]() {
        std::cout << "main semlock" << std::endl;
        std::cout << m_sharedMemory->GetData()->managerPID << std::endl;
        std::cout << m_sharedMemory->GetData()->cashierPID << std::endl;
        std::cout << m_sharedMemory->GetData()->restaurantPID << std::endl;

        std::cout << "Attractions: " << std::endl;
        for (int i = 0; i < ATTRACTION_COUNT; i++)
            std::cout << i << ": " << m_sharedMemory->GetData()->attractionPID[i] << std::endl;
    });

    sleep(1);
}

void MainProc::pInitImpl() {
    if (fork() == 0)
        execl("./park-manager", "park-manager", NULL);

    if (fork() == 0)
        execl("./park-cashier", "park-cashier", NULL);

    if (fork() == 0)
        execl("./park-restaurant", "park-restaurant", NULL);


    for (int i = 0; i < ATTRACTION_COUNT; i++)
        if (fork() == 0)
            execl("./park-attraction", "park-attraction", NULL);
}

void MainProc::pCloseImpl() {
    while(wait(NULL) > 0) { ; }

    m_sharedMemory->GetSemLock().Execute([this]() {
        std::cout << "main semlock" << std::endl;

        std::cout << m_sharedMemory->GetData()->managerPID << std::endl;
        std::cout << m_sharedMemory->GetData()->cashierPID << std::endl;
        std::cout << m_sharedMemory->GetData()->restaurantPID << std::endl;

        std::cout << "Attractions: " << std::endl;
        for (int i = 0; i < ATTRACTION_COUNT; i++) 
            std::cout << i << ": " << m_sharedMemory->GetData()->attractionPID[i] << std::endl;
    });
}