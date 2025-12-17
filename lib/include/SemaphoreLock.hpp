#pragma once
#include "SemaphoreArray.hpp"
#include <cstdint>
#include <sys/types.h>

struct SemaphoreLock {
    SemaphoreLock(SemaphoreArray::Semaphore& sem, uint16_t val = 1)
        :m_sem(sem), m_val(val)
    {
        m_sem->Wait(m_val);
    }

    ~SemaphoreLock() {
        m_sem->Signal(m_val);
    }

    SemaphoreLock(const SemaphoreLock&) = delete;
    SemaphoreLock& operator= (const SemaphoreLock&) = delete;
private:
    SemaphoreArray::Semaphore& m_sem;
    uint16_t m_val;
};