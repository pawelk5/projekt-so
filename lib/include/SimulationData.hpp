#pragma once
#include <array>
#include <cstdint>
#include <sys/types.h>

#define SHARED_MEMORY_PATH "/tmp/park_rozrywki_shm_main"
#define SHARED_MEMORY_KEY 'A'

#define MAIN_SEMAPHORE_ARRAY_PATH "/tmp/park_rozrywki_sem_main"
#define MAIN_SEMAPHORE_ARRAY_KEY 'B'

#define ATTRACTION_COUNT 17

enum class MainSemaphoreArray : uint16_t {
    MainSharedMemorySemaphore,
    Count
};

struct SimulationData {
    key_t sharedMemoryKey = 0;
    __pid_t managerPID = 0;
    __pid_t cashierPID = 0;
    __pid_t restaurantPID = 0;
    std::array<__pid_t, ATTRACTION_COUNT> attractionPID = { 0 };
};
