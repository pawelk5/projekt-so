#pragma once
#include <sys/types.h>
#include <memory>
#include <string>
#include <vector>


class SemaphoreArray {
public:
    struct SemaphoreStruct {
        friend SemaphoreArray;
        bool Wait(u_int16_t value = 1);
        bool Signal(u_int16_t value = 1);
        bool SetValue(int16_t value);
        int GetValue();

        SemaphoreStruct(SemaphoreStruct&& s) = default;
        SemaphoreStruct(SemaphoreArray& semArray, int semID);

    private:
        std::reference_wrapper<SemaphoreArray> m_semArray;
        int m_semID;
    };
    friend SemaphoreStruct;
    // Semaphore is a shared ptr to a semaphore struct !! CAN BE NULL
    typedef std::shared_ptr<SemaphoreStruct> Semaphore;

public:
    SemaphoreArray();
    ~SemaphoreArray();

    bool GetSemaphoreArray(const std::string& semPath, int semKey, u_int16_t nSems, bool create = false);
    bool DeleteSemaphoreArray();

    Semaphore GetSemaphore(u_int16_t semID);

protected:
    bool SemSignal(int semID, int16_t value);
    bool SemSetValue(int semID, int16_t value);
    int SemGetValue(int semID);
    
private:
    void pGenerateSemaphores(u_int16_t nSems);
    void pClearSemaphoreArray();

private:
    bool m_isOwner;
    std::vector<Semaphore> m_semaphores;

    struct SemData {
        key_t Key;
        int ID;

        bool isEmpty() { return Key <= 0 && ID <= 0; }
    };

    SemData m_semData;
};