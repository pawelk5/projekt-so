#include "RestaurantProc.hpp"

RestaurantProc::RestaurantProc() { ; }
RestaurantProc::~RestaurantProc() { ; }

RestaurantProc& RestaurantProc::Get() {
    static RestaurantProc app;
    return app;
}

void RestaurantProc::Run() {
    while (m_sharedMemory->GetData()->isOpen) {
        sleep(1);
    }
}

void RestaurantProc::pInitImpl() {
    m_sharedMemory->GetSemLock().Execute([this]() {
        if (!m_sharedMemory->GetData()->isOpen)
            throw std::runtime_error("park is closed!");
        
        if (m_sharedMemory->GetData()->attractionPID[RESTAURANT_INDEX] != 0)
            throw std::runtime_error("restaurant already exists!");

        m_sharedMemory->GetData()->attractionPID[RESTAURANT_INDEX] = getpid();
    });
}

void RestaurantProc::pCloseImpl() {
    m_sharedMemory->GetSemLock().Execute([this]() {    
        if (m_sharedMemory->GetData()->attractionPID[RESTAURANT_INDEX] == getpid())  
            m_sharedMemory->GetData()->attractionPID[RESTAURANT_INDEX] = 0;
    });
}