#include "MainProc.hpp"
#include <iostream>

MainProc::MainProc() { ; }
MainProc::~MainProc() { ; }

MainProc& MainProc::Get() {
    static MainProc app;
    return app;
}

void MainProc::Run() {
    m_sharedMemory->WithSemLock([this]() {
        std::cout << "main semlock" << std::endl;

        sleep(3);
        std::cout << m_sharedMemory->GetData()->managerPID << std::endl;
        std::cout << m_sharedMemory->GetData()->cashierPID << std::endl;
    });
    
    m_sharedMemory->WithSemLock([this]() {
        std::cout << "main semlock" << std::endl;

        std::cout << m_sharedMemory->GetData()->managerPID << std::endl;
        std::cout << m_sharedMemory->GetData()->cashierPID << std::endl;
    });
}

void MainProc::pInitImpl() {
    if (fork() == 0)
        execl("./park-manager", "park-manager", NULL);

    if (fork() == 0)
        execl("./park-cashier", "park-cashier", NULL);
}

void MainProc::pCloseImpl() {
    while(wait(NULL) > 0) { ; }

    m_sharedMemory->WithSemLock([this](){
        std::cout << m_sharedMemory->GetData()->managerPID << std::endl;
        std::cout << m_sharedMemory->GetData()->cashierPID << std::endl;
    });
}