#pragma once
#include <array>
#include <cstdint>
#include <sys/types.h>
#include "Config/Config.hpp"
#include "Config/AttractionData.hpp"

enum class MainSemaphoreArray : uint16_t {
    MainSharedMemorySemaphore,
    CashierEvent,
    // pause semaphores for attractions
    AttractionPause1,
    RestaurantPause = AttractionPause1 + ATTRACTION_COUNT - 1,

    // event counter semaphores for attractions
    AttractionEvent1,
    RestaurantEvent = AttractionEvent1 + ATTRACTION_COUNT - 1,

    // attraction handler semaphores (client leave semaphores)
    AttractionHandler1,
    RestaurantHandler = AttractionHandler1 + HandlerCount - 1,
    Count
};

struct SimulationData {
    uint16_t parkSize = 0;
    bool isOpen = 0;
    pid_t cashierPID = 0;
    pid_t mainPID = 0;
    std::array<__pid_t, ATTRACTION_COUNT> attractionPID = { 0 };
};
