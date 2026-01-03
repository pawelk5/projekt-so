#pragma once
#include <cstdint>
#include <variant>

enum class MainMQMessageType : int {
    END_SIMULATION
};

/// MAIN QUEUE
struct EndSimulation {};

using MainMQMessageContent = std::variant<EndSimulation>;

struct MainMQMessage {
    int senderPID;
    MainMQMessageType mType;

    MainMQMessageContent content;
};


/// CASHIER
struct EnterPark {
    bool hasChild;
    int childTID;
};

struct ExitPark {
    bool hasChild;
    int childTID;
};

struct EntryPermit {
    bool allowed;
};

struct Bill {
    float price;
};

enum class RegisterMessageType : int {
    ENTER_PARK,
    EXIT_PARK,
    ENTRY_PERMIT,
    BILL
};

using RegisterMessageContent = std::variant<EnterPark, ExitPark, EntryPermit, Bill>;

struct RegisterMQMessage {
    int senderPID;
    RegisterMessageType mType;
    
    RegisterMessageContent content;
};

