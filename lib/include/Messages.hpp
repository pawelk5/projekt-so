#pragma once
#include <variant>

enum class MainMQMessageType {
    END_SIMULATION
};

/// MAIN QUEUE
struct EndSimulation {};

typedef 
    std::variant<EndSimulation>
    MainQueueMessage;

struct MainMQMessage {
    int senderPID;
    int mType;

    MainQueueMessage msg;
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

typedef
    std::variant<EnterPark, ExitPark, EntryPermit, Bill>
    CashierMessage;