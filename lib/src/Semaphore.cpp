#include "SemaphoreArray.hpp"

SemaphoreArray::SemaphoreStruct::SemaphoreStruct(SemaphoreArray& semArray, int semID) 
    :m_semArray(semArray), m_semID(semID)
{

}

bool SemaphoreArray::SemaphoreStruct::Wait(u_int16_t value) { 
    return m_semArray.get().SemSignal(m_semID, -int16_t(value));
}
bool SemaphoreArray::SemaphoreStruct::Signal(u_int16_t value) {
    return m_semArray.get().SemSignal(m_semID, value);
}

bool SemaphoreArray::SemaphoreStruct::SetValue(int16_t value) {
    return m_semArray.get().SemSetValue(m_semID, value);
}

int SemaphoreArray::SemaphoreStruct::GetValue() {
    return m_semArray.get().SemGetValue(m_semID);
}