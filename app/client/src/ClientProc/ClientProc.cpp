#include "ClientProc.hpp"
#include "MessageTypes/ClientMQ.hpp"
#include "MessageTypes/RegisterMQ.hpp"
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
    auto reg = GetRegisterMQ(m_sharedMemory->GetData()->cashierPID);
    RegisterMQMessage message;
    message.mType = RegisterMessageType::ENTER_PARK;
    message.senderPID = getpid();
    message.content = EnterPark{ .hasChild=false, .childTID=-1 };

    reg->SendMessage(message);

    auto msg = m_clientQueue->RecieveMessage(10);
    if (msg) {
        if (msg->mType == ClientMessageType::ENTRY_PERMIT) {
            auto reply = std::get<EntryPermit>(msg->content);
            std::cout << "Klient " << getpid() << " otrzymal wiadomosc!\n"
                << "ENTRY PERMIT: " << reply.allowed << "\n";
        }
    }
}

void ClientProc::pInitImpl() {
    m_clientQueue = GetClientMQ(getpid(), true);
}

void ClientProc::pCloseImpl() {
    m_clientQueue = nullptr;
}