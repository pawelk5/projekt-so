#include "AttractionHandler.hpp"
#include "Utils.hpp"
#include <algorithm>
#include <numeric>
#include <ctime>
#include <utility>


AttractionHandler::AttractionHandler(const AttractionHandlerData& param) 
    :m_data(param)
{
    m_finishTime = 0;
    m_started = false;
}

AttractionHandler::~AttractionHandler() {
    /// signal the leave semaphore for all processes
    if (m_started)
        m_data.leaveSemaphore->Signal(m_data.clientList.size());
}

bool AttractionHandler::RemoveClient(pid_t pid) {
    return m_data.clientList.erase(pid) == 1;
}

time_t AttractionHandler::GetAttractionFinishTime() {
    return m_finishTime;
}

bool AttractionHandler::Finished() {
    return time(NULL) >= m_finishTime;
}

bool AttractionHandler::AddClient(pid_t pid, bool hasChild, bool fromPark) {
    if (Contains(m_data.clientList, pid))
        return false;

    m_data.clientList[pid] = __AttractionHandlerClientData{ .hasChild = hasChild, .fromPark = fromPark };
    return true;
}

void AttractionHandler::StartAttraction() {
    m_finishTime = time(NULL) + m_data.attractionDuration;
    m_data.leaveSemaphore->SetValue(0);
    m_started = true;
}

bool AttractionHandler::IsEmpty() {
    return m_data.clientList.size() == 0;
}

const AttractionHandlerData& AttractionHandler::GetHandlerData() {
    return m_data;
}

int AttractionHandler::GetClientCount() {
    return 
        std::accumulate(
            m_data.clientList.begin(),
            m_data.clientList.end(),
            0,
            [] (int sum, const std::pair<pid_t, __AttractionHandlerClientData>& clData) {
                return sum + 1 + clData.second.hasChild;
            }
        );
}