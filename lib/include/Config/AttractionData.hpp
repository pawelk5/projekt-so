#pragma once
#include <array>
#include <numeric>

struct __AttractionConfig {
    int duration;
    int maxClientsPerHandler;
    int handlerCount;

    bool canLeave;

};


constexpr std::array<__AttractionConfig, 17> AttractionConfig = {
    __AttractionConfig { // A1
        .duration=30, .maxClientsPerHandler=20,
        .handlerCount=1, .canLeave=true
    },
    __AttractionConfig { // A2
        .duration=30, .maxClientsPerHandler=12,
        .handlerCount=1, .canLeave=true
    },
    __AttractionConfig { // A3
        .duration=25, .maxClientsPerHandler=16,
        .handlerCount=1, .canLeave=true
    },
    __AttractionConfig { // A4
        .duration=35, .maxClientsPerHandler=18,
        .handlerCount=1, .canLeave=true
    },
    __AttractionConfig { // A5
        .duration=20, .maxClientsPerHandler=14,
        .handlerCount=1, .canLeave=true
    },
    __AttractionConfig { // A6
        .duration=20, .maxClientsPerHandler=8,
        .handlerCount=1, .canLeave=false
    },
    __AttractionConfig { // A7
        .duration=15, .maxClientsPerHandler=12,
        .handlerCount=1, .canLeave=false
    },
    __AttractionConfig { // A8
        .duration=15, .maxClientsPerHandler=24,
        .handlerCount=1, .canLeave=false
    },
    __AttractionConfig { // A9
        .duration=35, .maxClientsPerHandler=5,
        .handlerCount=4, .canLeave=false
    },
    /* A10 WAS NOT SPECIFIED */
    __AttractionConfig { // A10
        .duration=35, .maxClientsPerHandler=5,
        .handlerCount=4, .canLeave=false
    },/* A10 WAS NOT SPECIFIED */
    __AttractionConfig { // A11
        .duration=30, .maxClientsPerHandler=24,
        .handlerCount=1, .canLeave=false
    },
    __AttractionConfig { // A12
        .duration=25, .maxClientsPerHandler=18,
        .handlerCount=1, .canLeave=false
    },
    __AttractionConfig { // A13
        .duration=20, .maxClientsPerHandler=2,
        .handlerCount=4, .canLeave=true
    },
    __AttractionConfig { // A14
        .duration=20, .maxClientsPerHandler=3,
        .handlerCount=4, .canLeave=true
    },
    __AttractionConfig { // A15
        .duration=15, .maxClientsPerHandler=2,
        .handlerCount=10, .canLeave=true
    },
    __AttractionConfig { // A16
        .duration=35, .maxClientsPerHandler=9,
        .handlerCount=5, .canLeave=true
    },
    __AttractionConfig { // A17
        .duration=60, .maxClientsPerHandler=50,
        .handlerCount=1, .canLeave=true
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