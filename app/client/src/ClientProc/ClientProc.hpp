#pragma once
#include <sys/types.h>
#include <sys/wait.h>
#include "Process.hpp"
#include "MessageTypes/ClientMQ.hpp"
#include "MessageTypes/RegisterMQ.hpp"

class ClientProc : public Process {
public:
    static ClientProc& Get();
    void Run();

    ~ClientProc();

protected:
    ClientProc();
    void pInitImpl() override;
    void pCloseImpl() override;

    void pLeavePark();
    bool pEnterPark();

    bool pCreateReplyMQ();
private:
    ClientMQ m_clientQueue;
    bool m_enteredPark;

    struct ClientData {
        bool hasChild;
        bool isVip;
        TicketType ticketType;
    } m_data;

private:
    RegisterMQ m_registerMQ;
};