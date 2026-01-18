#include "ClientProc.hpp"
#include "Config/Config.hpp"
#include "MessageTypes/AttractionMQ.hpp"
#include "MessageTypes/ClientMQ.hpp"
#include "MessageTypes/RegisterMQ.hpp"
#include "PredefinedMQ.hpp"
#include "SimulationData.hpp"
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

ClientProc::ClientProc() { ; }
ClientProc::~ClientProc() { ; }

ClientProc& ClientProc::Get() {
    static ClientProc app;
    return app;
}

void ClientProc::Run() {
    if (!m_sharedMemory->GetData()->isOpen)
        return;
    
    if (!pCreateReplyMQ(GetClientParkMQ))
        return;
    m_enteredPark = pEnterPark();
    m_clientQueue = nullptr;
    if (!m_enteredPark)
        return;
    
    int parkTime = time(NULL) + (m_data.isVip ? RandomInt(60, 600) : TicketConfig.at(m_data.ticketType).time);
    std::vector<int> availableAttractions(AttractionConfig.size());
    std::iota(availableAttractions.begin(), availableAttractions.end(), 0);

    pLogMessage((std::string)"Klient" + (m_data.isVip ? " vip" : "") + " wchodzi do parku!");

    while (time(NULL) < parkTime && m_sharedMemory->GetData()->isOpen && availableAttractions.size() > 0) {
        try {
            int attractionID = availableAttractions.at(RandomInt(0, availableAttractions.size() - 1));
            const auto ct_attractionConfig = AttractionConfig.at(attractionID);
            if (attractionID == RESTAURANT_INDEX) {
                pCreateReplyMQ(GetClientRestaurantMQ);
                if (pVisitRestaurant())
                    m_visitedRestaurant = true;
            }
            else {
                pCreateAttractionReplyMQ((uint8_t)attractionID);
                auto semID = pEnterAttraction(attractionID);
                m_clientQueue = nullptr;

                if (semID != -1) {
                    // klient w atrakcji
                    pLogMessage("Klient wchodzi do atrakcji " + std::to_string(attractionID));
                    int attractionTime;
                    if (ct_attractionConfig.canLeave)
                        attractionTime = RandomInt(5, ct_attractionConfig.duration);
                    else
                        attractionTime = ct_attractionConfig.duration;
                    auto attractionSem = m_semaphoreArray->GetSemaphore(semID);

                    if (!attractionSem->Wait(1, true, false, attractionTime)){
                        pLogMessage("Klient wychodzi z atrakcji " + std::to_string(attractionID) + " (timeout)");
                        if (pCreateAttractionReplyMQ((uint8_t)attractionID))
                            pLeaveAttraction(attractionID);
                    } else {
                        pLogMessage("Klient wychodzi z atrakcji " + std::to_string(attractionID) + " (semop)");
                    }
                    
                    if (RandomChance(0.95)){
                        availableAttractions.erase(
                            std::find(availableAttractions.begin(),
                                availableAttractions.end(),
                                attractionID));
                    }
                }
            }
            m_attractionMQ = nullptr;
            m_restaurantMQ = nullptr;
            m_clientQueue = nullptr;
        } catch (std::exception e) {
            std::cerr << "Blad przy wchodzeniu do atrakcji!" << std::endl;
        }
    }

    pCreateReplyMQ(GetClientParkMQ);
    pLeavePark(m_visitedRestaurant);
    m_clientQueue = nullptr;
}

void ClientProc::pInitImpl() {
    m_enteredPark = false;
    m_visitedRestaurant = false;

    m_data.hasChild = RandomChance(CHILD_PROB);
    m_data.isVip = RandomChance(VIP_PROB);
    m_data.ticketType = m_data.isVip ? TicketType::VIP : (TicketType)RandomInt(0, (int)TicketType::H24);
    pSetProcessRole(ProcessRole::CLIENT);

    pLogMessage((std::string)"Klient" + (m_data.isVip ? " vip" : "") + " rozpoczyna prace!");
}

void ClientProc::pCloseImpl() {
    if (m_enteredPark){
        pCreateReplyMQ(GetClientParkMQ);
        pLeavePark(m_visitedRestaurant);
    }

    m_clientQueue = nullptr;
    m_attractionMQ = nullptr;
    m_registerMQ = nullptr;
    m_restaurantMQ = nullptr;
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

/// TODO
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
    }, true, 0, timeout ? CLIENT_MQ_TIMEOUT : -1);    
}

bool ClientProc::pSendAttractionMQMessage(const AttractionMQMessage& msg, bool timeout) {
    return m_attractionMQ->SendMessage(msg, [this] {
        if (errno == EBADF) {
            pLogMessage("BLAD: Atrakcja zostala zamknieta przed wyslaniem wiadomosci!");
            return true;
        }
        return false;
    }, true, 0, timeout ? CLIENT_MQ_TIMEOUT : -1);    
}