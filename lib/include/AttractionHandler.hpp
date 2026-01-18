#pragma once
#include "IPC/SemaphoreArray.hpp"
#include <map>

struct __AttractionHandlerClientData {
    bool hasChild;
    bool fromPark; /// Only used with restaurant
};

struct AttractionHandlerData {
    Semaphore leaveSemaphore;
    int maxCientCount;

    int attractionDuration;
    bool isRestaurant;
};


class AttractionHandler {
public:
    AttractionHandler(const AttractionHandlerData& param);
    ~AttractionHandler();

    bool RemoveClient(pid_t pid);
    time_t GetAttractionFinishTime();

    bool Finished();
    bool AddClient(pid_t pid, bool hasChild, bool fromPark = true);
    int GetClientCount();


    void StartAttraction();
    bool IsEmpty();


    AttractionHandler(const AttractionHandler&) = delete;
    AttractionHandler& operator= (const AttractionHandler&) = delete;
    
    const AttractionHandlerData& GetHandlerData();

private:
    std::map<pid_t, __AttractionHandlerClientData> m_clientList;
    AttractionHandlerData m_data;
    time_t m_finishTime;
    bool m_started;
};