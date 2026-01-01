#include "Process.hpp"
#include "Messages.hpp"
#include <memory>
#include <string>
#include <unistd.h>

void Process::Init(bool createIPC) {
    m_semaphoreArray = std::make_shared<SemaphoreArray>();
    m_sharedMemory = std::make_shared<SharedMemory<SimulationData>>();
    m_messageQueue = std::make_shared<MessageQueue<MainMQMessage>>();

    m_semaphoreArray->GetSemaphoreArray(MAIN_SEMAPHORE_ARRAY_PATH, MAIN_SEMAPHORE_ARRAY_KEY,
        (uint16_t)MainSemaphoreArray::Count, createIPC);
    
    m_sharedMemory->AttachMemory(SHARED_MEMORY_PATH, SHARED_MEMORY_KEY,
        m_semaphoreArray->GetSemaphore((uint16_t)MainSemaphoreArray::MainSharedMemorySemaphore), createIPC);
    
    MessageQueueParams t_msqParams;
    t_msqParams.create = true;
    t_msqParams.blocking = true;
    t_msqParams.msqName = std::to_string(getpid());
    t_msqParams.maxMsgCount = DEFAULT_MAX_MSQ_SIZE;

    m_messageQueue->OpenMessageQueue(t_msqParams);

    pInitImpl();
}

void Process::Close() {
    pCloseImpl();

    m_sharedMemory = nullptr;
    m_semaphoreArray = nullptr;
    m_messageQueue = nullptr;
}