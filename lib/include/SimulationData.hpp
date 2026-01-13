#pragma once
#include <array>
#include <cstdint>
#include <sys/types.h>
#include "Config/Config.hpp"


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
