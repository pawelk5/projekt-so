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

const std::map<TicketType, __TicketData> TicketConfig {
    {TicketType::H2, 
        __TicketData{
            .time=120,
            .price=20.f
    }},
    {TicketType::H4, 
        __TicketData{
            .time=240,
            .price=35.f
    }},
    {TicketType::H6, 
        __TicketData{
            .time=360,
            .price=50.f
    }},
    {TicketType::H24, 
        __TicketData{
            .time=1440,
            .price=150.f
    }},
    {TicketType::VIP, 
        __TicketData{
            .time=-1,
            .price=0.f
    }}
};