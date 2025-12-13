#pragma once
#include <array>
#include <sys/types.h>

struct SimulationData {
    key_t sharedMemoryKey = 0;
    __pid_t managerPID = 0;
    __pid_t cashierPID = 0;
    __pid_t restaurantPID = 0;
    std::array<__pid_t, 17> attractionPID = { 0 };
};
