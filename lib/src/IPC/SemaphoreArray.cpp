#include "IPC/SemaphoreArray.hpp"
#include "Utils.hpp"
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <iostream>
#include <unistd.h>

SemaphoreArray::SemaphoreArray() 
    :m_isOwner(false), m_semData(-1, -1)
{

}

SemaphoreArray::~SemaphoreArray() {
    DeleteSemaphoreArray();
}

bool SemaphoreArray::DeleteSemaphoreArray() {
    if (!m_isOwner || m_semData.isEmpty())
        return false;

    if (semctl(m_semData.ID, m_semaphores.size(), IPC_RMID) == -1) {
        perror("semctl (delete) error");
        throw std::runtime_error("Couldn't delete semaphore array!");
    }
    return true;
}

bool SemaphoreArray::GetSemaphoreArray(const std::string& semPath, int semKey, uint16_t nSems, bool create) {
    if (nSems == 0)
        return false;

    if (create)
        if (!CreateEmptyFile(semPath))
            throw std::runtime_error("Couldn't create semaphore array file!");

    if (!m_semaphores.empty())
        DeleteSemaphoreArray();

    if ((m_semData.Key = ftok(semPath.c_str(), semKey)) == -1) {
        perror("ftok (semget) error");
        throw std::runtime_error("Couldn't generate semaphore array key!");
        return false;
    }

    if ((m_semData.ID = semget(m_semData.Key, nSems, (create ? IPC_CREAT | IPC_EXCL : 0) | 0600)) == -1) {
        perror("semget error");
        throw std::runtime_error("Couldn't create semaphore array key!");
        return false;
    }
    m_isOwner = create;

    pGenerateSemaphores(nSems);
    return true;
}

void SemaphoreArray::pGenerateSemaphores(uint16_t nSems) {
    m_semaphores.clear();
    m_semaphores.reserve(nSems);

    for (int i = 0; i < nSems; i++)
        m_semaphores.push_back(
        std::make_shared<SemaphoreStruct>(
            (*this), i
        ));
}

bool SemaphoreArray::SemSignal(int semID, int16_t value, bool retryOnInterrupt, bool semundo, int timeout) {
    if (m_semData.isEmpty() || semID < 0 || semID >= m_semaphores.size() || timeout < -1)
        return false;
    
    sembuf action;
    action.sem_op = value;
    action.sem_num = semID;
    action.sem_flg = semundo ? SEM_UNDO : 0;

    bool leave = false;
    time_t endTime = time(NULL) + timeout;
    auto tmspc = timespec{ .tv_sec=endTime - time(NULL), .tv_nsec = 0 };

    while (true) {
        int result;
        if (timeout == -1)
            result = semop(m_semData.ID, &action, 1);
        else 
            result = semtimedop(m_semData.ID, &action, 1, &tmspc);

        if (result != -1)
            return true;


        if (endTime < time(NULL))
            return false;
        else
            tmspc.tv_sec = endTime - time(NULL);

        if (errno == EINTR) {
            if (!retryOnInterrupt)
                return false;
            else
                continue;
        }

        if (errno == EAGAIN)
            return false;
        
        break;
    }
    perror("semop error");
    return false;
}

bool SemaphoreArray::SemSetValue(int semID, int16_t value) {
    if (m_semData.isEmpty())
        return false;

    semun semopValue;
    semopValue.val = value;

    if (semctl(m_semData.ID, semID, SETVAL, semopValue) == -1) {
        perror("semctl (setval) error");
        return false;
    }
    return true;
}

int SemaphoreArray::SemGetValue(int semID) {
    if (m_semData.isEmpty() || semID < 0 || semID >= m_semaphores.size())
        return -1;

    int value = semctl(m_semData.ID, semID, GETVAL);
    if (value == -1)
        perror("semctl (getval) error");
    return value;
}

Semaphore SemaphoreArray::GetSemaphore(uint16_t semID) {
    if (m_semData.isEmpty())
        return nullptr;
    
    return semID >= m_semaphores.size() ? nullptr : m_semaphores.at(semID);
}