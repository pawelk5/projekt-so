#include "IPC/Signal.hpp"
#include <stdexcept>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>

bool CreateSignalHandler(int sig, void (*function)(int)) {
    struct sigaction sa;
    sa.sa_handler = function;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(sig, &sa, NULL) == -1) {
        perror("sigaction SIGUSR1");
        throw std::runtime_error("Couldn't set up signal handler!");
        return false;
    }
    return true;
}