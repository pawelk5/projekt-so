#include "Process.hpp"
#include <exception>
#include <memory>
#include <stdexcept>
#include <string>
#include <unistd.h>

void Process::Init(bool createIPC) {
    m_semaphoreArray = std::make_shared<SemaphoreArray>();
    m_sharedMemory = std::make_shared<SharedMemory<SimulationData>>();
    m_messageQueue = std::make_shared<MessageQueue<MainMQMessage>>();

    if (!m_semaphoreArray->GetSemaphoreArray(MAIN_SEMAPHORE_ARRAY_PATH, MAIN_SEMAPHORE_ARRAY_KEY,
        (uint16_t)MainSemaphoreArray::Count, createIPC))
        throw std::runtime_error("Couldn't create semaphore array!");
    
    if (!m_sharedMemory->AttachMemory(SHARED_MEMORY_PATH, SHARED_MEMORY_KEY,
        m_semaphoreArray->GetSemaphore((uint16_t)MainSemaphoreArray::MainSharedMemorySemaphore), createIPC))
        throw std::runtime_error("Couldn't attach shared memory!");
    
    MessageQueueParams t_msqParams;
    t_msqParams.create = true;
    t_msqParams.blocking = true;
    t_msqParams.msqName = std::to_string(getpid());
    t_msqParams.maxMsgCount = DEFAULT_MAX_MSQ_SIZE;

    if (!m_messageQueue->OpenMessageQueue(t_msqParams))
        throw std::runtime_error("Couldn't open message queue!");

    pInitImpl();
}

void Process::Close() {
    try {
    pCloseImpl();
    } catch (const std::exception& e) {
        m_sharedMemory = nullptr;
        m_semaphoreArray = nullptr;
        m_messageQueue = nullptr;
        throw;
    }
    m_sharedMemory = nullptr;
    m_semaphoreArray = nullptr;
    m_messageQueue = nullptr;
}

std::shared_ptr<SemaphoreArray> Process::GetSemaphoreArray() {
    return m_semaphoreArray;
}

std::shared_ptr<SharedMemory<SimulationData>> Process::GetSharedMemory() {
    return m_sharedMemory;
}

MainMessageQueue Process::GetMainMessageQueue() {
    return m_messageQueue;
}