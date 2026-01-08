#include "SemaphoreArray.hpp"
#include "Utils.hpp"
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>

SemaphoreArray::SemaphoreArray() 
    :m_isOwner(false), m_semData(0, 0)
{

}

SemaphoreArray::~SemaphoreArray() {
    DeleteSemaphoreArray();
}

bool SemaphoreArray::DeleteSemaphoreArray() {
    if (!m_isOwner || m_semData.isEmpty() || m_semData.ID <= 0)
        return false;

    if (semctl(m_semData.ID, m_semaphores.size(), IPC_RMID) == -1) {
        perror("semctl (delete) error");
        throw std::runtime_error("Couldn't delete semaphore array!");
    }
    return true;
}

bool SemaphoreArray::GetSemaphoreArray(const std::string& semPath, int semKey, u_int16_t nSems, bool create) {
    if (nSems == 0)
        return false;

    if (create)
        if (!CreateEmptyFile(semPath))
            throw std::runtime_error("Couldn't create semaphore array file!");

    if (!m_semaphores.empty() && m_isOwner)
        DeleteSemaphoreArray();

    if ((m_semData.Key = ftok(semPath.c_str(), semKey)) == -1) {
        perror("ftok (semget) error");
        throw std::runtime_error("Couldn't generate semaphore array key!");
    }

    if ((m_semData.ID = semget(m_semData.Key, nSems, (create ? IPC_CREAT | IPC_EXCL : 0) | 0666)) == -1) {
        perror("semget error");
        // semaphore array doesnt exist
        if (errno == ENOENT)
            return false;
        throw std::runtime_error("Couldn't create semaphore array key!");
    }

    m_isOwner = create;

    pGenerateSemaphores(nSems);
    return true;
}

void SemaphoreArray::pGenerateSemaphores(u_int16_t nSems) {
    m_semaphores.clear();
    m_semaphores.reserve(nSems);

    for (int i = 0; i < nSems; i++)
        m_semaphores.push_back(
        std::make_shared<SemaphoreStruct>(
            (*this), i
        ));
}

bool SemaphoreArray::SemSignal(int semID, int16_t value, bool retryOnInterrupt, bool semundo) {
    if (m_semData.ID == 0 || semID < 0 || semID >= m_semaphores.size())
        return false;

    sembuf action;
    action.sem_op = value;
    action.sem_num = semID;
    action.sem_flg = semundo ? SEM_UNDO : 0;

    bool leave = false;
    do {
        if (semop(m_semData.ID, &action, 1) == -1) {
            if (errno == EINTR && retryOnInterrupt)
                continue;
            perror("semop error");
            return false;
        } 
        leave = true;
    } while (!leave);
    return true;
}

bool SemaphoreArray::SemSetValue(int semID, int16_t value) {
    if (semctl(m_semData.ID, semID, SETVAL, value) == -1) {
        perror("semctl (setval) error");
        return false;
    }
    return true;
}

int SemaphoreArray::SemGetValue(int semID) {
    int value = semctl(m_semData.ID, semID, GETVAL);
    if (value == -1)
        perror("semctl (getval) error");
    return value;
}

SemaphoreArray::Semaphore SemaphoreArray::GetSemaphore(u_int16_t semID) {
    return semID >= m_semaphores.size() ? nullptr : m_semaphores.at(semID);
}