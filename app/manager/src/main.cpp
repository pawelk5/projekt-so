#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include "SemaphoreArray.hpp"
#include "SharedMemory.hpp"
#include "SimulationData.hpp"

int main() {
    SemaphoreArray semaphoreArray;
    semaphoreArray.GetSemaphoreArray(MAIN_SEMAPHORE_ARRAY_PATH, MAIN_SEMAPHORE_ARRAY_KEY, 1);

    SharedMemory<SimulationData> sharedMemory;
    sharedMemory.AttachMemory(SHARED_MEMORY_PATH, SHARED_MEMORY_KEY,
        semaphoreArray.GetSemaphore(0));
        
    if (sharedMemory.GetData()->managerPID != 0)
        return -1;

    {
        auto t_semlock = sharedMemory.GetSemLock();

        std::cout << "manager semlock" << std::endl;
        sharedMemory.GetData()->managerPID = getpid();
    }

    sleep(1);
    {
        auto t_semlock = sharedMemory.GetSemLock();
        std::cout << "manager semlock" << std::endl;
        
        sharedMemory.GetData()->managerPID = 0;
    }

    return 0;
}