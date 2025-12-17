#pragma once
#include <string>
#include <iostream>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "SemaphoreArray.hpp"
#include "Utils.hpp"

// TODO: error detection
template<class T>
class SharedMemory {
public:
    struct SharedMemoryData {
        key_t Key;
        key_t ID;

        bool isEmpty() {
            return Key == 0 && ID == 0;
        }
    };

public:
    SharedMemory() 
        :m_memPtr(nullptr), m_memData(0, 0), m_isOwner(false)
    {

    }
    ~SharedMemory() { pDetachMemory(); }

    bool AttachMemory(const std::string& shmPath, int shmKey, SemaphoreArray::Semaphore shmSemaphore = nullptr, bool create = false) {
        return pAttachMemory(shmPath, shmKey, shmSemaphore, create);
    }
    
    bool DetachMemory() { return pDetachMemory(); }

    SemaphoreArray::Semaphore GetSemaphore() { return m_sem; }
    T* const GetData() { return m_memPtr; }

private:
    bool pDetachMemory() {
        if (!m_memPtr)
            return true;

        int result = shmdt((void* const) m_memPtr);
        // todo: check for errors
        m_memPtr = nullptr;

        // if the object created shared memory then it will automatically remove it when it detaches it
        if (m_isOwner)
            pDeleteMemory();
        
        m_memData = { 0, 0 };
        return true;
    }

    void pDeleteMemory() {
        shmctl(m_memData.ID, IPC_RMID, nullptr);
        // todo: check for errors

        m_isOwner = false;
        m_memData = { 0, 0 };
        m_memPtr = nullptr;
    }

    bool pAttachMemory(const std::string& shmPath, int shmKey, SemaphoreArray::Semaphore shmSemaphore, bool create) {
        if (create)
            if (!CreateEmptyFile(shmPath))
                return false;

        if (!m_memData.isEmpty())
            pDetachMemory();

        m_memData.Key = ftok(shmPath.c_str(), shmKey);
        m_memData.ID = shmget(m_memData.Key, sizeof(T), IPC_CREAT | (IPC_EXCL && create) | 0666);
        m_isOwner = create;

        // attach pointer
        m_memPtr = (T*)shmat(m_memData.ID, nullptr, IPC_CREAT | 0666);
        m_sem = shmSemaphore;
        if (m_isOwner)
            m_sem->SetValue(1);
        
        return true;
    }

private:
    T* m_memPtr;
    SharedMemoryData m_memData;
    bool m_isOwner;

    SemaphoreArray::Semaphore m_sem;
};