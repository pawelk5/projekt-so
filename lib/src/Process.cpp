#include "Process.hpp"
#include <memory>

void Process::Init(bool createIPC) {
    m_semaphoreArray = std::make_shared<SemaphoreArray>();
    m_sharedMemory = std::make_shared<SharedMemory<SimulationData>>();

    m_semaphoreArray->GetSemaphoreArray(MAIN_SEMAPHORE_ARRAY_PATH, MAIN_SEMAPHORE_ARRAY_KEY,
        (uint16_t)MainSemaphoreArray::Count, createIPC);
    
    m_sharedMemory->AttachMemory(SHARED_MEMORY_PATH, SHARED_MEMORY_KEY,
        m_semaphoreArray->GetSemaphore((uint16_t)MainSemaphoreArray::MainSharedMemorySemaphore), createIPC);

    pInitImpl();
}

void Process::Close() {
    pCloseImpl();

    m_semaphoreArray = nullptr;
    m_sharedMemory = nullptr;
}