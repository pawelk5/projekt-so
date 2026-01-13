#pragma once
#include "IPC/SemaphoreArray.hpp"
#include "IPC/SharedMemory.hpp"
#include "MessageTypes/LoggerMQ.hpp"
#include "SimulationData.hpp"
#include <memory>


/// Base process class
/// Automatically handles main semaphore array and shared memory.
/// Provides an interface for creating logs
class Process {
public:
    Process() = default;
    virtual ~Process() = default;

    /// Initializes the process (attaches semaphore array and shared memory)
    /// \throws std::runtime_error if initialization fails
    /// \param createIPC if true the process will be the owner of IPCs and will be responsible for creating/destroying them
    void Init(bool createIPC = false);
    void Close();

    Process(const Process&) = delete;
    Process& operator=(const Process&) = delete;

    std::shared_ptr<SemaphoreArray> GetSemaphoreArray();
    std::shared_ptr<SharedMemory<SimulationData>> GetSharedMemory();

protected:
    /// Process specific initialization (called after attaching/ shared memory and semaphore array)
    virtual void pInitImpl() = 0;

    /// Process specific close function (called before detaching/removing shared memory and semaphore array)
    virtual void pCloseImpl() = 0;

    /// Log message to a file (through logger service)
    /// \param message message to be logged
    /// \param backupLogToStdout if true message will be printed to stdout if saving to file fails
    /// \param endWorkMessage if true the function will send special message signaling end of work for logger service
    void pLogMessage(const std::string& message, bool backupLogToStdout = true, bool endWorkMessage = false);

    /// Sets process role, used with logging
    void pSetProcessRole(ProcessRole role);

    std::shared_ptr<SemaphoreArray> m_semaphoreArray;
    std::shared_ptr<SharedMemory<SimulationData>> m_sharedMemory;

private:
    Semaphore m_loggerSemaphore;
    ProcessRole m_processRole;
};