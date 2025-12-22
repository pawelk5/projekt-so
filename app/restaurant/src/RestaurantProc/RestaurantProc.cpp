#include "RestaurantProc.hpp"

RestaurantProc::RestaurantProc() { ; }
RestaurantProc::~RestaurantProc() { ; }

RestaurantProc& RestaurantProc::Get() {
    static RestaurantProc app;
    return app;
}

void RestaurantProc::Run() {
    sleep(2);
}

void RestaurantProc::pInitImpl() {
    m_sharedMemory->GetSemLock().Execute([this]() {
        if (m_sharedMemory->GetData()->restaurantPID != 0)
            throw std::runtime_error("restaurant already exists!");

        m_sharedMemory->GetData()->restaurantPID = getpid();
    });
}

void RestaurantProc::pCloseImpl() {
    m_sharedMemory->GetSemLock().Execute([this]() {    
        if (m_sharedMemory->GetData()->restaurantPID == getpid())  
            m_sharedMemory->GetData()->restaurantPID = 0;
    });
}