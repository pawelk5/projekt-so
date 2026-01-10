#pragma once
#include <array>
#include <cstdint>
#include <sys/types.h>

#define SHARED_MEMORY_PATH "/tmp/park_rozrywki_shm_main"
#define SHARED_MEMORY_KEY 'A'

#define MAIN_SEMAPHORE_ARRAY_PATH "/tmp/park_rozrywki_sem_main"
#define MAIN_SEMAPHORE_ARRAY_KEY 'B'

#define ATTRACTION_COUNT 17
#define RESTAURANT_INDEX 16
#define PARK_SIZE 100

enum class MainSemaphoreArray : uint16_t {
    MainSharedMemorySemaphore,
    CashierLoop,
    Attraction1Loop,
    RestaurantLoop = 17,
    Count
};

struct SimulationData {
    uint16_t parkSize = 0;
    bool isOpen = 0;
    pid_t cashierPID = 0;
    pid_t mainPID = 0;
    std::array<__pid_t, ATTRACTION_COUNT> attractionPID = { 0 };
};
