#pragma once


struct EmptyMessage { };

struct ParkEntryPermit {
    bool allowed;
};

struct AttractionEntryPermit {
    bool allowed;
    unsigned short leaveSemaphoreID;
};

struct Bill {
    float price;
};