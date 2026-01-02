#pragma once
#include <cstdint>
#include <variant>

enum class MainMQMessageType : int {
    END_SIMULATION
};

/// MAIN QUEUE
struct EndSimulation {};

struct MainMQMessage {
    int senderPID;
    MainMQMessageType mType;

    union content {
        EndSimulation endSimulation;
    } msg;
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

struct RegisterMQMessage {
    int senderPID;
    RegisterMessageType mType;
    
    union content {
        EnterPark enterPark;
        ExitPark exitPark;
        EntryPermit entryPermit;
        Bill bill;
    } msg;
};

