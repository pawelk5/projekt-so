#include "SemaphoreArray.hpp"
#include "Utils.hpp"
#include <cstdint>
#include <memory>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>

SemaphoreArray::SemaphoreArray() 
    :m_isOwner(false), m_semData(0, 0)
{

}

SemaphoreArray::~SemaphoreArray() {
    if (m_isOwner)
        DeleteSemaphoreArray();
}

bool SemaphoreArray::DeleteSemaphoreArray() {
    if (!m_isOwner)
        return false;

    semctl(m_semData.ID, m_semaphores.size(), IPC_RMID);
    return true;
}

bool SemaphoreArray::GetSemaphoreArray(const std::string& semPath, int semKey, u_int16_t nSems, bool create) {
    if (nSems == 0)
        return false;

    if (create)
        if (!CreateEmptyFile(semPath))
            return false;

    if (!m_semaphores.empty() && m_isOwner)
        DeleteSemaphoreArray();

    m_semData.Key = ftok(semPath.c_str(), semKey);
    m_semData.ID = semget(m_semData.Key, nSems, IPC_CREAT | (IPC_EXCL && create) | 0666);
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

bool SemaphoreArray::SemSignal(int semID, int16_t value) {
    if (m_semData.ID == 0 || semID < 0 || semID >= m_semaphores.size())
        return false;

    sembuf action;
    action.sem_op = value;
    action.sem_num = semID;
    action.sem_flg = 0;

    semop(m_semData.ID, &action, 1);
    return true;
}

bool SemaphoreArray::SemSetValue(int semID, int16_t value) {
    semctl(m_semData.ID, semID, SETVAL, value);
    return true;
}

int SemaphoreArray::SemGetValue(int semID) {
    return semctl(m_semData.ID, semID, GETVAL);
}

SemaphoreArray::Semaphore SemaphoreArray::GetSemaphore(u_int16_t semID) {
    if (semID >= m_semaphores.size())
        return nullptr;
    return m_semaphores.at(semID);
}