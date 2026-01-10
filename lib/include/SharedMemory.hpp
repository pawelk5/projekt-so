#pragma once
#include <stdexcept>
#include <string>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "SemaphoreArray.hpp"
#include "SemaphoreLock.hpp"
#include "Utils.hpp"

// TODO: error detection
template<class T>
class SharedMemory {
public:
    struct SharedMemoryData {
        key_t Key;
        key_t ID;

        bool isEmpty() { return Key <= 0 && ID <= 0; }
    };

public:
    SharedMemory() 
        :m_memPtr(nullptr), m_memData(0, 0), m_isOwner(false)
    {

    }
    ~SharedMemory() { pDetachMemory(); }

    bool AttachMemory(const std::string& shmPath, int shmKey, Semaphore shmSemaphore = nullptr, bool create = false) {
        return pAttachMemory(shmPath, shmKey, shmSemaphore, create);
    }
    
    bool DetachMemory() { return pDetachMemory(); }

    SemaphoreLock GetSemLock() { return SemaphoreLock(m_sem); }

    Semaphore GetSemaphore() { return m_sem; }
    T* const GetData() { return m_memPtr; }

private:
    bool pDetachMemory() {
        if (!m_memPtr)
            return true;

        int result = shmdt((void* const) m_memPtr);
        // todo: check for errors ?
        m_memPtr = nullptr;

        // if the object created shared memory then it will automatically remove it when it detaches it
        if (m_isOwner)
            pDeleteMemory();
        
        m_memData = { 0, 0 };
        return true;
    }

    void pDeleteMemory() {
        if (auto result = shmctl(m_memData.ID, IPC_RMID, nullptr)) {
            perror("shmctl (delete) error");
            throw std::runtime_error("Couldn't delete shared memory!");
        }
        m_isOwner = false;
        m_memData = { 0, 0 };
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
        }
        if ( (m_memData.ID = shmget(m_memData.Key, sizeof(T), (create ? IPC_CREAT | IPC_EXCL : 0) | 0666)) == -1 ) {
            perror("shmget error");
            if (errno == ENOENT)
                return false;
            throw std::runtime_error("Couldn't allocate shared memory!");
        }
        
        m_isOwner = create;

        // attach pointer
        if (!(m_memPtr = (T*)shmat(m_memData.ID, nullptr, 0))) {
            perror("shmat error");
            throw std::runtime_error("Couldn't attach shared memory!");
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