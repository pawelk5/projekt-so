#pragma once
#include <sys/types.h>
#include <memory>
#include <string>
#include <vector>

/// Semaphore array class
/// Automatically handles detaching and removing the semaphore array
class SemaphoreArray {
public:
    /// Struct representation of a single semaphore
    struct SemaphoreStruct {
        friend SemaphoreArray;
        /// Waits the semaphore
        /// \param value value to be subtracted from semaphore
        /// \param retryOnInterrupt if the function is interrupted by a signal the function will retry the semaphore operation
        /// \param semundo perform semsignal with SEM_UNDO flag
        /// \returns false if an error occurs
        bool Wait(uint16_t value = 1, bool retryOnInterrupt = false, bool semundo = false);

        /// Signals a semaphore
        /// \param value value to be added (or subtracted) from semaphore
        /// \returns false if an error occurs
        bool Signal(uint16_t value = 1);

        /// Sets semaphore value
        /// \param value new semaphore value
        /// \returns false if an error occurs
        bool SetValue(int16_t value);

        /// Gets semaphore value
        /// \returns semaphore value, -1 if an error occured
        int GetValue();

        SemaphoreStruct(SemaphoreStruct&& s) = default;
        SemaphoreStruct(SemaphoreArray& semArray, int semID);

    private:
        std::reference_wrapper<SemaphoreArray> m_semArray;
        int m_semID;
    };
    friend SemaphoreStruct;

public:
    SemaphoreArray();
    ~SemaphoreArray();
    
    /// Creates or attaches the semaphore array
    /// \param semPath path of the semaphore array file (used with ftok)
    /// \param semKey semaphore array key
    /// \param nSems number of semaphores
    /// \param create if true, semaphore array object will be responsible for creating and removing the semaphore array
    /// \throws std::runtime_error if an error occurs
    bool GetSemaphoreArray(const std::string& semPath, int semKey, uint16_t nSems, bool create = false);

    /// Creates or attaches the semaphore array
    /// \returns false if semaphore array object is not attached to any semaphore array
    /// \throws std::runtime_error if an error occurs
    bool DeleteSemaphoreArray();

    /// Gets semaphore struct pointer
    /// \param semID ID (index) of semaphore in semaphore array
    /// \returns pointer to the semaphore struct, or nullptr if ID is out of range
    std::shared_ptr<SemaphoreStruct> GetSemaphore(uint16_t semID);

protected:
    /// Signals a semaphore, or waits if the value is negative
    /// \param semID semaphore ID (index in semaphore array)
    /// \param value value to be added (or subtracted) from semaphore
    /// \param retryOnInterrupt if the function is interrupted by a signal the function will retry the semaphore operation
    /// \param semundo perform semsignal with SEM_UNDO flag
    /// \returns false if an error occurs
    bool SemSignal(int semID, int16_t value, bool retryOnInterrupt, bool semundo);

    /// Sets semaphore value
    /// \param semID semaphore ID (index in semaphore array)
    /// \param value new semaphore value
    /// \returns false if an error occurs
    bool SemSetValue(int semID, int16_t value);

    /// Gets semaphore value
    /// \param semID semaphore ID (index in semaphore array)
    /// \returns semaphore value, -1 if an error occured
    int SemGetValue(int semID);
    
private:
    void pGenerateSemaphores(uint16_t nSems);
    void pClearSemaphoreArray();

private:
    bool m_isOwner;
    std::vector<std::shared_ptr<SemaphoreStruct>> m_semaphores;

    struct SemData {
        key_t Key;
        int ID;

        bool isEmpty() { return Key <= 0 && ID <= 0; }
    };

    SemData m_semData;
};

/// Semaphore is a shared ptr to a semaphore struct 
/// CAN BE NULL
using Semaphore = std::shared_ptr<SemaphoreArray::SemaphoreStruct>;