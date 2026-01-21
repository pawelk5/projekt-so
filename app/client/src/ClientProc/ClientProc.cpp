#include "ClientProc.hpp"
#include "Config/Config.hpp"
#include "MessageTypes/AttractionMQ.hpp"
#include "MessageTypes/ClientMQ.hpp"
#include "MessageTypes/RegisterMQ.hpp"
#include "MessageTypes/RestaurantMQ.hpp"
#include "PredefinedMQ.hpp"
#include "SimulationData.hpp"
#include "IPC/Signal.hpp"
#include "Utils.hpp"
#include <algorithm>
#include <cstdio>
#include <ctime>
#include <exception>
#include <string>
#include <sys/types.h>
#include <iostream>
#include <unistd.h>
#include <vector>

static volatile bool evac = false; 

void SigUsr1Handler(int sig) {
    evac = true;
    ClientProc::Get().SetEvacFlag(evac);
}

ClientProc::ClientProc() { ; }
ClientProc::~ClientProc() { ; }

ClientProc& ClientProc::Get() {
    static ClientProc app;
    return app;
}

void ClientProc::pInitImpl() {
    if (!CreateSignalHandler(SIGUSR1, SigUsr1Handler))
        throw std::runtime_error("couldn't create sigusr1 handler!");
    
    m_enteredPark = m_evac = false;
    m_visitedRestaurant = false;

    pGenerateClientData();
    pSetProcessRole(ProcessRole::CLIENT);

    pLogMessage((std::string)"Klient" + (m_data.isVip ? " vip" : "") + " rozpoczyna prace!");
}

void ClientProc::Run() {
    if (!m_sharedMemory->GetData()->isOpen)
        return;
    
    if (RandomChance(0.1f))
        pVisitRestaurant();
    pRemoveAllMQs();

    if (!pCreateReplyMQ(GetClientParkMQ))
        return;
    m_enteredPark = pEnterPark();
    pRemoveAllMQs();

    if (!m_enteredPark)
        return;
    
    int parkTime = time(NULL) + (m_data.isVip ? RandomInt(60, 600) : TicketConfig.at(m_data.ticketType).time);
    std::vector<int> availableAttractions(AttractionConfig.size());
    std::iota(availableAttractions.begin(), availableAttractions.end(), 0);

    pLogMessage((std::string)"Klient" + (m_data.isVip ? " vip" : "") + " wchodzi do parku!");

    while (time(NULL) < parkTime && m_sharedMemory->GetData()->isOpen && availableAttractions.size() > 0 && !m_evac) {
        int attractionID = availableAttractions.at(RandomInt(0, availableAttractions.size() - 1));
        try {
            if (attractionID == RESTAURANT_INDEX) {
                if (pVisitRestaurant())
                    m_visitedRestaurant = true;
            }
            else {
                pVisitAttraction(attractionID);
            }
        } catch (std::exception e) {
            std::cerr << "Blad przy wchodzeniu do atrakcji!" << std::endl;
        }
        if (RandomChance(0.95)) {
            availableAttractions.erase(
        std::find(availableAttractions.begin(),
            availableAttractions.end(),
            attractionID));
        }

        pRemoveAllMQs();
    }
    
    if (!m_data.isVip)
        pCreateReplyMQ(GetClientParkMQ);

    pLeavePark(m_visitedRestaurant);
    pRemoveAllMQs();

    if (RandomChance(0.1f))
        pVisitRestaurant();
    pRemoveAllMQs();
}

void ClientProc::pCloseImpl() {
    if (m_enteredPark){
        pCreateReplyMQ(GetClientParkMQ);
        pLeavePark(m_visitedRestaurant);
    }

    pRemoveAllMQs();
    pLogMessage("Klient konczy prace!");
}

bool ClientProc::pCreateReplyMQ(std::function<ClientMQ(pid_t, bool, const std::function<bool()>&)> func) {
    m_clientQueue = func(getpid(), true, [this] {
        if (errno == ENOSPC) {
            pLogMessage("BLAD: Za duzo kolejek komunikatow w systemie!");
            return true;
        }
        return false;
    });

    return m_clientQueue != nullptr;
}


bool ClientProc::pCreateAttractionReplyMQ(short attractionID) {
    m_clientQueue = GetClientAttractionMQ(getpid(), attractionID, true, [this] {
        if (errno == ENOSPC) {
            pLogMessage("BLAD: Za duzo kolejek komunikatow w systemie!");
            return true;
        }
        return false;
    });

    return m_clientQueue != nullptr;
}

bool ClientProc::pGetRegisterMQ(bool blocking) {
    m_registerMQ = GetRegisterMQ(m_sharedMemory->GetData()->cashierPID, false, [this] {
        if (errno == ENOENT) {
            pLogMessage("BLAD: Kasa zostala zamknieta przed wyslaniem wiadomosci!");
            return true;
        }

        return false;
    }, blocking);

    return m_registerMQ != nullptr;
}

bool ClientProc::pGetAttractionMQ(short attractionID, bool blocking) {
    m_attractionMQ = GetAttractionMQ(m_sharedMemory->GetData()->attractionPID.at(attractionID), false, [this] {
        if (errno == ENOENT) {
            pLogMessage("BLAD: Atrakcja zostala zamknieta przed wyslaniem wiadomosci!");
            return true;
        }
        return false;
    }, blocking);

    return m_attractionMQ != nullptr;
}

bool ClientProc::pGetRestaurantMQ(bool blocking) {
    m_restaurantMQ = GetRestaurantMQ(m_sharedMemory->GetData()->attractionPID.at(RESTAURANT_INDEX), false, [this] {
        if (errno == ENOENT) {
            pLogMessage("BLAD: Atrakcja zostala zamknieta przed wyslaniem wiadomosci!");
            return true;
        }
        return false;
    }, blocking);

    return m_restaurantMQ != nullptr;
}


bool ClientProc::pSendRegisterMQMessage(const RegisterMQMessage& msg, bool timeout) {
    return m_registerMQ->SendMessage(msg, [this] {
        if (errno == EBADF) {
            pLogMessage("BLAD: Kasa zostala zamknieta przed wyslaniem wiadomosci!");
            return true;
        }
        return false;
    }, true, m_data.isVip, timeout ? CLIENT_MQ_TIMEOUT : -1);    
}

bool ClientProc::pSendAttractionMQMessage(const AttractionMQMessage& msg, bool timeout) {
    return m_attractionMQ->SendMessage(msg, [this] {
        if (errno == EBADF) {
            pLogMessage("BLAD: Atrakcja zostala zamknieta przed wyslaniem wiadomosci!");
            return true;
        }
        return false;
    }, !timeout, 0, timeout ? CLIENT_MQ_TIMEOUT : -1);    
}

bool ClientProc::pSendRestaurantMQMessage(const RestaurantMQMessage& msg, bool timeout) {
    return m_restaurantMQ->SendMessage(msg, [this] {
        if (errno == EBADF) {
            pLogMessage("BLAD: Atrakcja zostala zamknieta przed wyslaniem wiadomosci!");
            return true;
        }
        return false;
    }, !timeout, 0, timeout ? CLIENT_MQ_TIMEOUT : -1);    
}

void ClientProc::SetEvacFlag(bool flag) {
    m_evac = flag;
    if (m_evac)
        pLogMessage((std::string)"Klient otrzymuje sygnal ewakuacji!");
}

void ClientProc::pRemoveAllMQs() {
    m_clientQueue = nullptr;
    m_attractionMQ = nullptr;
    m_registerMQ = nullptr;
    m_restaurantMQ = nullptr;
}

void ClientProc::pGenerateClientData() {
    m_data.isVip = RandomChance(VIP_PROB);
    m_data.ticketType = m_data.isVip ? TicketType::VIP : (TicketType)RandomInt(0, (int)TicketType::H24);

    m_data.personData.age = RandomInt(14, 90);
    m_data.personData.height = RandomInt(130, 200);

    if (m_data.personData.age >= 18)
        m_data.hasChild = RandomChance(CHILD_PROB);
    else 
        m_data.hasChild = false;

    if (!m_data.hasChild) {
        m_data.childData = { 0, 0 };
        return;
    }

    m_data.childData.age = RandomInt(1, 13);
    m_data.childData.height = RandomInt(50, 170);
}