#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include "SharedMemory.hpp"
#include "SimulationData.hpp"

const char* SHARED_MEMORY_PATH = "/tmp/park_rozrywki_main";
const int SHARED_MEMORY_KEY = 'A';

int main() {
    SharedMemory<SimulationData> sharedMemory = 
        SharedMemory<SimulationData>(SHARED_MEMORY_PATH, SHARED_MEMORY_KEY);

    sharedMemory.GetData()->managerPID = getpid();
    sleep(6);
    sharedMemory.GetData()->managerPID = 0;
    return 0;
}