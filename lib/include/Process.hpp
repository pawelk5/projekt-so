#pragma once
#include "MessageQueue.hpp"
#include "SemaphoreArray.hpp"
#include "SharedMemory.hpp"
#include "SimulationData.hpp"
#include <memory>

class Process {
public:
    Process() = default;
    virtual ~Process() = default;

    void Init(bool createIPC = false);
    void Close();

    Process(const Process&) = delete;
    Process& operator=(const Process&) = delete;

    std::shared_ptr<SemaphoreArray> GetSemaphoreArray();
    std::shared_ptr<SharedMemory<SimulationData>> GetSharedMemory();

protected:
    virtual void pInitImpl() = 0;
    virtual void pCloseImpl() = 0;
    void pLogMessage(const std::string& message, bool backupLogToStdout = true, bool endWorkMessage = false);

    std::shared_ptr<SemaphoreArray> m_semaphoreArray;
    std::shared_ptr<SharedMemory<SimulationData>> m_sharedMemory;

private:
    SemaphoreArray::Semaphore m_loggerSemaphore;
};