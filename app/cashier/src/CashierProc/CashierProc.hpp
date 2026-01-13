#pragma once
#include <memory>
#include <map>
#include <sys/types.h>
#include <sys/wait.h>
#include <vector>
#include "MessageTypes/ClientMQ.hpp"
#include "MessageTypes/RegisterMQ.hpp"
#include "Process.hpp"
#include "PredefinedMQ.hpp"

class CashierProc : public Process {
public:
    static CashierProc& Get();
    void Run();

    ~CashierProc();

protected:
    CashierProc();
    void pInitImpl() override;
    void pCloseImpl() override;

private:
    void pHandleRegisterMQ();
    void pHandleEnterPark(const RegisterMQMessage& message);
    void pHandleExitPark(const RegisterMQMessage& message);
    bool pRegisterClient(const RegisterMQMessage& message);

    float pCalculatePrice(pid_t pid);
    bool pCreateReplyMQ(pid_t pid);
    bool pSendReply(pid_t pid, const ClientMQMessage& msg);
private:
    RegisterMQ m_registerQueue;
    ClientMQ m_replyMQ;
    Semaphore m_loopSemaphore;

private:
    struct ClientData {
        bool hasChild;
        bool vip;

        time_t entryTime;
        TicketType ticketType;
    };
    std::map<pid_t, ClientData> m_clients;
    int m_clientCounter;

    std::vector<RegisterMQMessage> m_enterVipQueue;
    std::vector<RegisterMQMessage> m_enterQueue;

private:
    void pRemoveClient(pid_t pid);
};