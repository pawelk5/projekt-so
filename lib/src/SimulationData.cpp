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

bool MeetsAttractionCriteria(int attractionID, const PersonData& client, const PersonData& child, bool hasChild) {
    const auto attractionConfig = AttractionConfig.at(attractionID);

    if (attractionConfig.maxHeight != -1 && client.height > attractionConfig.maxHeight)
        return false;

    if (attractionConfig.minHeight != -1 && client.height < attractionConfig.minHeight)
        return false;

    if (hasChild) {
        if (attractionConfig.minAge != -1 && child.age < attractionConfig.minAge)
            return false;

        if (attractionConfig.minChildHeight != -1 && child.height < attractionConfig.minChildHeight)
            return false;
        else if (attractionConfig.minHeight != -1 && child.height < attractionConfig.minHeight)
            return false;
    }

    return true;
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