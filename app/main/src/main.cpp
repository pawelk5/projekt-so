#include <cstddef>
#include <iostream>
#include <sys/wait.h>
#include "SharedMemory.hpp"
#include "SimulationData.hpp"

int main() {
    SemaphoreArray semaphoreArray;
    semaphoreArray.GetSemaphoreArray(MAIN_SEMAPHORE_ARRAY_PATH, MAIN_SEMAPHORE_ARRAY_KEY,
        1, true);
    
    SharedMemory<SimulationData> sharedMemory; 
    sharedMemory.AttachMemory(SHARED_MEMORY_PATH, SHARED_MEMORY_KEY,
        semaphoreArray.GetSemaphore(0), true);

    if (fork() == 0)
        execl("./park-manager", "park-manager", NULL);

    {
        auto t_semlock = sharedMemory.GetSemLock();
        std::cout << "main semlock" << std::endl;

        sleep(3);
        std::cout << sharedMemory.GetData()->managerPID << std::endl;
    }
    
    {
        auto t_semlock = sharedMemory.GetSemLock();
        std::cout << "main semlock" << std::endl;

        std::cout << sharedMemory.GetData()->managerPID << std::endl;
    }

    while(wait(NULL) > 0);

    {
        auto t_semlock = sharedMemory.GetSemLock();
        std::cout << "main semlock" << std::endl;

        std::cout << sharedMemory.GetData()->managerPID << std::endl;
    }
    
    return 0;
}