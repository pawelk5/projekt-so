#include "AttractionProc.hpp"
#include "SimulationData.hpp"

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

void AttractionProc::Run() {
    sleep(2);
}

void AttractionProc::pInitImpl() {
    m_sharedMemory->GetSemLock().Execute([this]() {
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
            throw std::runtime_error("restaurant already exists!");        
    });
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