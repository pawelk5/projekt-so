#pragma once
#include <memory>
#include <optional>
#include <sys/types.h>
#include <sys/wait.h>
#include "AttractionHandler.hpp"
#include "Process.hpp"
#include "IPC/SemaphoreArray.hpp"
#include "MessageTypes/RestaurantMQ.hpp"
#include "MessageTypes/ClientMQ.hpp"
#include "SimulationData.hpp"

class RestaurantProc : public Process {
public:
    static RestaurantProc& Get();
    void Run();

    ~RestaurantProc();
    void CloseAttraction();
    void OpenAttraction();
    void HandleSigint();

protected:
    RestaurantProc();
    void pInitImpl() override;
    void pCloseImpl() override;

private:
    void pHandleRestaurantMQ();
    void pSignalClients();

    void pCreateAttractionHandler();
    bool pRegisterClient(const RestaurantMQMessage& message, std::shared_ptr<AttractionHandler> handler);

    bool pSendReply(pid_t pid, const ClientMQMessage& msg);
    bool pCreateReplyMQ(pid_t pid);

    void pHandleEnterRestaurant(const RestaurantMQMessage& message);
    void pSendBill(pid_t pid);
    void pRemoveClient(pid_t pid);

    time_t pGetNextTimeout();

private:
    Semaphore m_pauseSemaphore;
    Semaphore m_eventSemaphore;
    Semaphore m_handlerSemaphore;

    RestaurantMQ m_restaurantMQ;
    ClientMQ m_replyMQ;

    std::vector<RestaurantMQMessage> m_enterQueue;
    std::shared_ptr<AttractionHandler> m_handler;
    std::map<pid_t, bool> m_clients;

private:
    __AttractionConfig cm_attractionConfig;
};