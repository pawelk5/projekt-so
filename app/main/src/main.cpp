#include <cstddef>
#include <iostream>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "SharedMemory.hpp"
#include "SimulationData.hpp"

const char* SHARED_MEMORY_PATH = "/tmp/park_rozrywki_main";
const int SHARED_MEMORY_KEY = 'A';

int main() {
    
    SharedMemory<SimulationData> sharedMemory; 
    sharedMemory.AttachMemory(SHARED_MEMORY_PATH, SHARED_MEMORY_KEY, true);

    std::cout << sharedMemory.GetData()->managerPID << std::endl;
    if (fork() == 0)
        execl("./park-manager", "park-manager", NULL);

    sleep(4);
    std::cout << sharedMemory.GetData()->managerPID << std::endl;
    
    while(wait(NULL) > 0);
    std::cout << sharedMemory.GetData()->managerPID << std::endl;

    return 0;
}