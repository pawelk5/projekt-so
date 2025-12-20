#pragma once
#include <sys/types.h>
#include <sys/wait.h>
#include "Process.hpp"

class AttractionProc : public Process {
public:
    static AttractionProc& Get();
    void Run();

    ~AttractionProc();
    
    int GetAttractionID();
protected:
    AttractionProc();
    void pInitImpl() override;
    void pCloseImpl() override;

private:
    int m_attractionID;
};