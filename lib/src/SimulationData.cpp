#include "SimulationData.hpp"

int GetFirstHandlerSemaphoreID(size_t attractionIndex) {
    if (attractionIndex > 17)
        return (int) -1;

    uint16_t offset = 
        std::accumulate(
            AttractionConfig.begin(),
            AttractionConfig.begin() + attractionIndex,
            0,
            [] (uint16_t sum, const __AttractionConfig& cfg) {
                return sum + cfg.handlerCount;
            }
        );

    return (int) ((int)MainSemaphoreArray::AttractionHandler1 + offset);
}