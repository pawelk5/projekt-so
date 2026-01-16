#include "AttractionHandler.hpp"
#include "Utils.hpp"
#include <algorithm>
#include <ctime>


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
    auto it = std::find(
        m_data.clientList.begin(),
        m_data.clientList.end(),
        pid
    );

    if (it != m_data.clientList.end()) {
        m_data.clientList.erase(it);
        return true;
    }

    return false;
}

time_t AttractionHandler::GetAttractionFinishTime() {
    return m_finishTime;
}

bool AttractionHandler::Finished() {
    return time(NULL) >= m_finishTime;
}

bool AttractionHandler::AddClient(pid_t pid) {
    if (Contains(m_data.clientList, pid))
        return false;

    m_data.clientList.push_back(pid);
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