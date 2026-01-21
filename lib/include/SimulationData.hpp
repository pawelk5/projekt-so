#pragma once
#include <array>
#include <cstdint>
#include <map>
#include <sys/types.h>
#include "Config/Config.hpp"
#include <numeric>

struct __AttractionConfig {
    uint8_t duration;
    uint8_t maxClientsPerHandler;
    uint8_t handlerCount;

    bool canLeave;

    int minChildHeight;
    int minHeight;
    int minAge;

    int maxHeight = -1;
};

struct PersonData {
    uint8_t age;
    uint8_t height;
};

constexpr std::array<__AttractionConfig, 17> AttractionConfig = {
    __AttractionConfig { // A1
        .duration=30, .maxClientsPerHandler=20,
        .handlerCount=1, .canLeave=true,

        .minChildHeight=120,
        .minHeight=-1, .minAge=-1
    },
    __AttractionConfig { // A2
        .duration=30, .maxClientsPerHandler=12,
        .handlerCount=1, .canLeave=true,

        .minChildHeight=100,
        .minHeight=120, .minAge=-1
    },
    __AttractionConfig { // A3
        .duration=25, .maxClientsPerHandler=16,
        .handlerCount=1, .canLeave=true,

        .minChildHeight=-1,
        .minHeight=120, .minAge=2
    },
    __AttractionConfig { // A4
        .duration=35, .maxClientsPerHandler=18,
        .handlerCount=1, .canLeave=true,

        .minChildHeight=110,
        .minHeight=135, .minAge=-1
    },
    __AttractionConfig { // A5
        .duration=20, .maxClientsPerHandler=14,
        .handlerCount=1, .canLeave=true,

        .minChildHeight=-1,
        .minHeight=130, .minAge=2
    },
    __AttractionConfig { // A6
        .duration=20, .maxClientsPerHandler=8,
        .handlerCount=1, .canLeave=false,

        .minChildHeight=-1,
        .minHeight=120, .minAge=-1
    },
    __AttractionConfig { // A7
        .duration=15, .maxClientsPerHandler=12,
        .handlerCount=1, .canLeave=false,

        .minChildHeight=-1,
        .minHeight=130, .minAge=2,
        .maxHeight=190
    },
    __AttractionConfig { // A8
        .duration=15, .maxClientsPerHandler=24,
        .handlerCount=1, .canLeave=false,

        .minChildHeight=100,
        .minHeight=120, .minAge=-1
    },
    __AttractionConfig { // A9
        .duration=35, .maxClientsPerHandler=5,
        .handlerCount=4, .canLeave=false,

        .minChildHeight=-1,
        .minHeight=120, .minAge=4
    },
    /* A10 WAS NOT SPECIFIED */
    __AttractionConfig { // A10
        .duration=35, .maxClientsPerHandler=5,
        .handlerCount=4, .canLeave=false,

        .minChildHeight=-1,
        .minHeight=120, .minAge=4
    },/* A10 WAS NOT SPECIFIED */
    __AttractionConfig { // A11
        .duration=30, .maxClientsPerHandler=24,
        .handlerCount=1, .canLeave=false,

        .minChildHeight=-1,
        .minHeight=140, .minAge=-1,
        .maxHeight=195
    },
    __AttractionConfig { // A12
        .duration=25, .maxClientsPerHandler=18,
        .handlerCount=1, .canLeave=false,

        .minChildHeight=-1,
        .minHeight=140, .minAge=-1,
        .maxHeight=195
    },
    __AttractionConfig { // A13
        .duration=20, .maxClientsPerHandler=2,
        .handlerCount=4, .canLeave=true,

        .minChildHeight=-1,
        .minHeight=140, .minAge=-1,
        .maxHeight=195
    },
    __AttractionConfig { // A14
        .duration=20, .maxClientsPerHandler=3,
        .handlerCount=4, .canLeave=true,

        .minChildHeight=-1,
        .minHeight=130, .minAge=4
    },
    __AttractionConfig { // A15
        .duration=15, .maxClientsPerHandler=2,
        .handlerCount=10, .canLeave=true,

        .minChildHeight=-1,
        .minHeight=120, .minAge=-1
    },
    __AttractionConfig { // A16
        .duration=35, .maxClientsPerHandler=9,
        .handlerCount=5, .canLeave=true,

        .minChildHeight=120,
        .minHeight=140, .minAge=-1,
        .maxHeight=195
    },
    __AttractionConfig { // A17
        .duration=60, .maxClientsPerHandler=50,
        .handlerCount=1, .canLeave=true,

        .minChildHeight=-1,
        .minHeight=-1, .minAge=-1
    }
};

constexpr int HandlerCount = 
    std::accumulate(
        AttractionConfig.begin(),
        AttractionConfig.end(),
        0,
        [] (int sum, const __AttractionConfig& cfg) {
            return sum + cfg.handlerCount;
        }
    );

enum class MainSemaphoreArray : uint16_t {
    MainSharedMemorySemaphore,
    CashierEvent,
    // pause semaphores for attractions
    AttractionPause1,
    MainPause = AttractionPause1 + ATTRACTION_COUNT - 1,

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

/// Returns the semaphore ID of the first handler associated with an attraction
/// \param attractionIndex index of the attraction
int GetFirstHandlerSemaphoreID(size_t attractionIndex);

enum class TicketType : int {
    H2,
    H4,
    H6,
    H24,    
    VIP
};

struct __TicketData {
    int time;
    float price;
};

extern const std::map<TicketType, __TicketData> TicketConfig;