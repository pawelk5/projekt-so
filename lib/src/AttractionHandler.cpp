#include "AttractionHandler.hpp"
#include "Utils.hpp"
#include <algorithm>
#include <numeric>
#include <ctime>
#include <utility>
#include <iostream>

AttractionHandler::AttractionHandler(const AttractionHandlerData& param) 
    :m_data(param)
{
    m_data.leaveSemaphore->SetValue(0);
    m_finishTime = 0;
    m_started = m_released = false;
}

AttractionHandler::~AttractionHandler() {
    ReleaseClients();
}

bool AttractionHandler::RemoveClient(pid_t pid) {
    return m_clientList.erase(pid) == 1;
}

time_t AttractionHandler::GetAttractionFinishTime() {
    return m_finishTime;
}

bool AttractionHandler::Finished() {
    return time(NULL) >= m_finishTime || IsEmpty();
}

bool AttractionHandler::AddClient(pid_t pid, bool hasChild, bool fromPark) {
    if (m_clientList.contains(pid))
        return false;

    m_clientList[pid] = __AttractionHandlerClientData{ .hasChild = hasChild, .fromPark = fromPark };
    return true;
}

void AttractionHandler::StartAttraction() {
    if (m_started)
        return;

    m_finishTime = time(NULL) + m_data.attractionDuration;
    m_data.leaveSemaphore->SetValue(0);
    m_started = true;
}

bool AttractionHandler::IsEmpty() {
    return m_clientList.empty();
}

const AttractionHandlerData& AttractionHandler::GetHandlerData() {
    return m_data;
}

int AttractionHandler::GetClientCount() {
    return 
        std::accumulate(
            m_clientList.begin(),
            m_clientList.end(),
            0,
            [] (int sum, const std::pair<pid_t, __AttractionHandlerClientData>& clData) {
                return sum + 1 + clData.second.hasChild;
            }
        );
}

const std::map<pid_t, __AttractionHandlerClientData>& AttractionHandler::GetClientList() {
    return m_clientList;
}

void AttractionHandler::ReleaseClients() {
    if (m_released)
        return;

    m_data.leaveSemaphore->Signal(m_clientList.size());
    m_released = true;
}