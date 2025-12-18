#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include "SemaphoreArray.hpp"
#include "SharedMemory.hpp"
#include "SimulationData.hpp"

int main() {
    SemaphoreArray semaphoreArray;
    semaphoreArray.GetSemaphoreArray(MAIN_SEMAPHORE_ARRAY_PATH, MAIN_SEMAPHORE_ARRAY_KEY,
        (uint16_t)MainSemaphoreArray::Count);

    SharedMemory<SimulationData> sharedMemory;
    sharedMemory.AttachMemory(SHARED_MEMORY_PATH, SHARED_MEMORY_KEY,
        semaphoreArray.GetSemaphore((uint16_t)MainSemaphoreArray::MainSharedMemorySemaphore));
    
    {
        sharedMemory.WithSemLock([&sharedMemory]() {
            if (sharedMemory.GetData()->managerPID != 0)
                throw std::runtime_error("manager already exists!");

            sharedMemory.GetData()->managerPID = getpid();
        });
    }

    sleep(1);
    {
        auto t_semlock = sharedMemory.GetSemLock();
        std::cout << "manager semlock" << std::endl;
        
        sharedMemory.GetData()->managerPID = 0;
    }

    return 0;
}