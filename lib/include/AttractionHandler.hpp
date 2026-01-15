#pragma once
#include "IPC/SemaphoreArray.hpp"

struct AttractionHandlerData {
    Semaphore leaveSemaphore;
    int maxCientCount;

    int attractionDuration;
    std::vector<pid_t> clientList;
};


class AttractionHandler {
public:
    AttractionHandler(const AttractionHandlerData& param);
    ~AttractionHandler();

    bool RemoveClient(pid_t pid);
    time_t GetAttractionFinishTime();

    bool Finished();
    bool AddClient(pid_t pid);
    
    void StartAttraction();
    bool IsEmpty();

    AttractionHandler(const AttractionHandler&) = delete;
    AttractionHandler& operator= (const AttractionHandler&) = delete;
    
    const AttractionHandlerData& GetHandlerData();

private:
    AttractionHandlerData m_data;
    time_t m_finishTime;
    bool m_started;
};