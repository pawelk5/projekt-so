#include "ClientProc.hpp"
#include "Messages.hpp"
#include "PredefinedMQ.hpp"
#include <iostream>
#include <unistd.h>

ClientProc::ClientProc() { ; }
ClientProc::~ClientProc() { ; }

ClientProc& ClientProc::Get() {
    static ClientProc app;
    return app;
}

void ClientProc::Run() {
    auto reg = GetCashierMQ(m_sharedMemory->GetData()->cashierPID);
    RegisterMQMessage message;
    message.mType = RegisterMessageType::ENTER_PARK;
    message.senderPID = getpid();
    message.msg.enterPark = EnterPark{ .hasChild=false, .childTID=-1 };

    reg->SendMessage(message);
}

void ClientProc::pInitImpl() {
    //std::cout << "Klient (" << getpid() << ") zaczyna dzialanie!" << std::endl;
}

void ClientProc::pCloseImpl() {
    //std::cout << "Klient (" << getpid() << ") konczy dzialanie!" << std::endl;
}