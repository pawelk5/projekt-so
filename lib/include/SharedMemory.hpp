#pragma once
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "Utils.hpp"

// TODO: error detection
template<class T>
class SharedMemory {
public:
    struct SharedMemoryData {
        key_t Key;
        key_t ID;
    };

public:
    SharedMemory(const char* shmPath, int shmKey) 
        :m_isOwner(true)
    {
        pCreateSharedMemory(shmPath, shmKey);
        pAttachMemory();
    }

    SharedMemory(const SharedMemoryData& data)
        :m_isOwner(false), m_memData(data)
    {
        pAttachMemory();
    }

    ~SharedMemory() {
        pDetachMemory();
        if (m_isOwner)
            pDeleteMemory();
    }

    T* const GetData() {
        return m_memPtr;
    }

private:
    T* m_memPtr;
    SharedMemoryData m_memData;
    bool m_isOwner;

private:
    void pAttachMemory() {
        // TODO: check if memPtr is not null
        auto* m_memPtr = (T*) shmat(m_memData.ID, nullptr, IPC_CREAT | 0666);
    }

    void pDetachMemory() {
        int result = shmdt((void* const) m_memPtr);
        m_memPtr = nullptr;
    }

    void pDeleteMemory() {
        shmctl(m_memData.ID, IPC_RMID, nullptr);
        m_memPtr = nullptr;
    }

    void pCreateSharedMemory(const char* shmPath, int shmKey) {
        if (!CreateEmptyFile(shmPath))
            return;

        m_memData.Key = ftok(shmPath, shmKey);
        m_memData.ID = shmget(m_memData.Key, sizeof(T), IPC_CREAT | IPC_EXCL | 0666);
    }
};