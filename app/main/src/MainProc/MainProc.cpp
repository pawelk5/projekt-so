#include "MainProc.hpp"
#include <iostream>

MainProc::MainProc() { ; }
MainProc::~MainProc() { ; }

MainProc& MainProc::Get() {
    static MainProc app;
    return app;
}

void MainProc::Run() {
    {
        auto t_semlock = m_sharedMemory->GetSemLock();
        std::cout << "main semlock" << std::endl;

        sleep(3);
        std::cout << m_sharedMemory->GetData()->managerPID << std::endl;
    }
    
    {
        auto t_semlock = m_sharedMemory->GetSemLock();
        std::cout << "main semlock" << std::endl;

        std::cout << m_sharedMemory->GetData()->managerPID << std::endl;
    }
}

void MainProc::pInitImpl() {
    if (fork() == 0)
        execl("./park-manager", "park-manager", NULL);

    if (fork() == 0)
        execl("./park-manager", "park-manager", NULL);
}

void MainProc::pCloseImpl() {
    while(wait(NULL) > 0) { ; }

    {
        auto t_semlock = m_sharedMemory->GetSemLock();
        std::cout << "main semlock" << std::endl;

        std::cout << m_sharedMemory->GetData()->managerPID << std::endl;
    }
}