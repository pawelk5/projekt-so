#include "Process.hpp"
#include "MessageTypes/LoggerMQ.hpp"
#include "PredefinedMQ.hpp"
#include "SimulationData.hpp"
#include "Utils.hpp"
#include <cstddef>
#include <exception>
#include <memory>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include <signal.h>

void Process::Init(bool createIPC) {
    /// prevent process from exiting when sigpipe occurs
    signal(SIGPIPE, SIG_IGN);

    m_semaphoreArray = std::make_shared<SemaphoreArray>();
    m_sharedMemory = std::make_shared<SharedMemory<SimulationData>>();
    
    if (!m_semaphoreArray->GetSemaphoreArray(MAIN_SEMAPHORE_ARRAY_PATH, MAIN_SEMAPHORE_ARRAY_KEY,
        (uint16_t)MainSemaphoreArray::Count, createIPC))
        throw std::runtime_error("Couldn't create semaphore array!");
    
    if (!m_sharedMemory->AttachMemory(SHARED_MEMORY_PATH, SHARED_MEMORY_KEY,
        m_semaphoreArray->GetSemaphore((uint16_t)MainSemaphoreArray::MainSharedMemorySemaphore), createIPC))
        throw std::runtime_error("Couldn't attach shared memory!");

    m_processRole = ProcessRole::MAIN;
    pInitImpl();
}

void Process::Close() {
    try {
        pCloseImpl();
    } catch (const std::exception& e) {
        m_sharedMemory = nullptr;
        m_semaphoreArray = nullptr;
        throw;
    }
    m_sharedMemory = nullptr;
    m_semaphoreArray = nullptr;
}

std::shared_ptr<SemaphoreArray> Process::GetSemaphoreArray() {
    return m_semaphoreArray;
}

std::shared_ptr<SharedMemory<SimulationData>> Process::GetSharedMemory() {
    return m_sharedMemory;
}

void Process::pLogMessage(const std::string& message, bool backupLogToStdout, bool endWorkMessage) {
    auto mq = GetLoggerMQ(m_sharedMemory->GetData()->mainPID, false);

    LoggerMQMessage msg;
    msg.senderPID = endWorkMessage ? -1 : getpid();
    msg.senderRole = m_processRole;
    msg.timestamp = time(NULL);
    msg.message = ToArray<LOGGER_MESSAGE_MAX_LENGTH>(message);
    mq->SendMessage(msg);
}

void Process::pSetProcessRole(ProcessRole role) {
    m_processRole = role;
}