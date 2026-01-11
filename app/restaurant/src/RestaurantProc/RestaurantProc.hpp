#pragma once
#include <sys/types.h>
#include <sys/wait.h>
#include "Process.hpp"
#include "SemaphoreArray.hpp"

class RestaurantProc : public Process {
public:
    static RestaurantProc& Get();
    void Run();

    ~RestaurantProc();
    void CloseAttraction();
    void OpenAttraction();
protected:
    RestaurantProc();
    void pInitImpl() override;
    void pCloseImpl() override;

private:
    Semaphore m_restaurantSemaphore;
};