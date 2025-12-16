#pragma once
#include <string>
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

        bool isEmpty() {
            return Key == 0 && ID == 0;
        }
    };

public:
    SharedMemory() 
        :m_memPtr(nullptr), m_memData(0, 0), m_isOwner(false)
    {

    }

    bool AttachMemory(const std::string& shmPath, int shmKey, bool create = false) {
        return pAttachMemory(shmPath, shmKey, create);
    }
    
    bool DetachMemory() {
        return pDetachMemory();
    }

    ~SharedMemory() {
        pDetachMemory();
    }

    T* const GetData() {
        return m_memPtr;
    }

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

    bool pAttachMemory(const std::string& shmPath, int shmKey, bool create) {
        if (!CreateEmptyFile(shmPath))
            return false;

        if (!m_memData.isEmpty())
            pDetachMemory();

        m_memData.Key = ftok(shmPath.c_str(), shmKey);
        m_memData.ID = shmget(m_memData.Key, sizeof(T), IPC_CREAT | (IPC_EXCL && create) | 0666);
        m_isOwner = create;

        // attach pointer
        m_memPtr = (T*)shmat(m_memData.ID, nullptr, IPC_CREAT | 0666);
        return true;
    }

private:
    T* m_memPtr;
    SharedMemoryData m_memData;
    bool m_isOwner;
};