#pragma once
#include "SemaphoreArray.hpp"
#include <cstdint>
#include <sys/types.h>

struct SemaphoreLock {
    explicit SemaphoreLock(Semaphore& sem, uint16_t val = 1)
        :m_sem(sem), m_val(val), m_released(false)
    {
        m_sem->Wait(m_val, true, true);
    }

    // try to execute and automatically release semlock
    template<class Callable>
    void Execute(Callable function) {
        try {
            function();
        }
        catch (const std::exception& e) {
            Release();
            throw;
        }
    }

    ~SemaphoreLock() {
        Release();
    }

    void Release() {
        if (!m_released)
            m_sem->Signal(m_val);
        m_released = true;
    }

    SemaphoreLock(const SemaphoreLock&) = delete;
    SemaphoreLock& operator= (const SemaphoreLock&) = delete;
private:
    Semaphore& m_sem;
    uint16_t m_val;

    bool m_released;
};