#pragma once
#include <stdexcept>
#include <string>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "SemaphoreArray.hpp"
#include "SemaphoreLock.hpp"
#include "Utils.hpp"

/// Shared memory class template
/// Automatically handles detaching and removing the shared memory
template<class T>
class SharedMemory {
public:
    struct SharedMemoryData {
        key_t Key;
        key_t ID;

        bool isEmpty() { return Key < 0 && ID < 0; }
    };

public:
    SharedMemory() 
        :m_memPtr(nullptr), m_memData(-1, -1), m_isOwner(false)
    {

    }
    ~SharedMemory() { pDetachMemory(); }

    /// Creates or attaches the shared memory
    /// \param shmPath path of the shared memory file (used with ftok)
    /// \param shmKey shared memory key
    /// \param shmSemaphore semaphore for synchronizing read and write operations
    /// \param create if true, shared memory object will be responsible for creating and removing the shared memory
    /// \returns false on failure
    /// \throws std::runtime_error if an error occurs
    bool AttachMemory(const std::string& shmPath, int shmKey, Semaphore shmSemaphore = nullptr, bool create = false) {
        return pAttachMemory(shmPath, shmKey, shmSemaphore, create);
    }
    
    /// Detaches and removes the shared memory
    /// \returns false on failure
    /// \throws std::runtime_error if an error occurs
    bool DetachMemory() { return pDetachMemory(); }

    /// Returns a semaphore lock object for synchronizing read and write operations
    SemaphoreLock GetSemLock() { return SemaphoreLock(m_sem); }

    /// Returns semaphore used for synchronizing read and write operations
    Semaphore GetSemaphore() { return m_sem; }

    /// Returns pointer to shared memory
    /// \returns pointer to shared memory managed by this object
    T* const GetData() { return m_memPtr; }

private:
    bool pDetachMemory() {
        if (!m_memPtr)
            return true;

        if (shmdt((void* const) m_memPtr) == -1) {
            perror("shmdt error");
            throw std::runtime_error("Couldn't detach shared memory!");
            return false;
        }

        m_memPtr = nullptr;

        // if the object created shared memory then it will automatically remove it when it detaches it
        if (m_isOwner)
            pDeleteMemory();
        
        m_memData = { -1, -1 };
        return true;
    }

    void pDeleteMemory() {
        if (auto result = shmctl(m_memData.ID, IPC_RMID, nullptr)) {
            perror("shmctl (delete) error");
            throw std::runtime_error("Couldn't delete shared memory!");
        }
        m_isOwner = false;
        m_memData = { -1, -1 };
        m_memPtr = nullptr;
    }

    bool pAttachMemory(const std::string& shmPath, int shmKey, Semaphore shmSemaphore, bool create) {
        if (create)
            if (!CreateEmptyFile(shmPath))
                throw std::runtime_error("Couldn't create shared memory file!");

        if (!m_memData.isEmpty())
            pDetachMemory();

        if ( (m_memData.Key = ftok(shmPath.c_str(), shmKey)) == -1 ){
            perror("ftok (shmget) error");
            throw std::runtime_error("Couldn't generate shared memory key!");
            return false;
        }
        if ( (m_memData.ID = shmget(m_memData.Key, sizeof(T), (create ? IPC_CREAT | IPC_EXCL : 0) | 0666)) == -1 ) {
            perror("shmget error");
            throw std::runtime_error("Couldn't allocate shared memory!");
            return false;
        }
        
        m_isOwner = create;

        // attach pointer
        if (!(m_memPtr = (T*)shmat(m_memData.ID, nullptr, 0))) {
            perror("shmat error");
            throw std::runtime_error("Couldn't attach shared memory!");
            return false;
        }

        m_sem = shmSemaphore;
        if (m_isOwner)
            m_sem->SetValue(1);
        
        return true;
    }

private:
    T* m_memPtr;
    SharedMemoryData m_memData;
    bool m_isOwner;

    Semaphore m_sem;
};