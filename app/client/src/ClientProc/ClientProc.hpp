#pragma once
#include <functional>
#include <sys/types.h>
#include <sys/wait.h>
#include "Process.hpp"
#include "MessageTypes/ClientMQ.hpp"
#include "MessageTypes/RegisterMQ.hpp"
#include "MessageTypes/AttractionMQ.hpp"
#include "MessageTypes/RestaurantMQ.hpp"

class ClientProc : public Process {
public:
    static ClientProc& Get();
    void Run();

    ~ClientProc();

    void SetEvacFlag(bool flag);

protected:
    ClientProc();
    void pInitImpl() override;
    void pCloseImpl() override;

    void pLeavePark(bool visitedRestaurant);
    bool pEnterPark();

    // park entry, restaurant
    bool pCreateReplyMQ(std::function<ClientMQ(pid_t, bool, const std::function<bool()>&)> func);
    bool pCreateAttractionReplyMQ(short attractionID);
    bool pGetRegisterMQ(bool blocking);
    bool pGetRestaurantMQ(bool blocking);
    bool pGetAttractionMQ(short attractionID, bool blocking);

    bool pSendRegisterMQMessage(const RegisterMQMessage& msg, bool timeout);
    bool pSendAttractionMQMessage(const AttractionMQMessage& msg, bool timeout);
    bool pSendRestaurantMQMessage(const RestaurantMQMessage& msg, bool timeout);
    
    /// Returns wait semaphore or -1
    int pEnterAttraction(int attractionID);
    /// Only when leaving mid-attraction
    void pLeaveAttraction(int attractionID);

    void pVisitAttraction(int attractionID);

    bool pVisitRestaurant();
    bool pEnterRestaurant();
    void pLeaveRestaurant();

    void pRemoveAllMQs();
    
private:
    ClientMQ m_clientQueue;
    bool m_enteredPark;
    bool m_visitedRestaurant;
    bool m_evac;
    
    struct ClientData {
        bool hasChild;
        bool isVip;
        TicketType ticketType;
    } m_data;

private:
    RegisterMQ m_registerMQ;
    AttractionMQ m_attractionMQ;
    RestaurantMQ m_restaurantMQ;
};