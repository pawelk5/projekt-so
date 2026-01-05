#pragma once
#include "MessageTypes/RegisterMQ.hpp"
#include "MessageTypes/ClientMQ.hpp"

RegisterMQ GetRegisterMQ(pid_t cashierPID, bool cashier = false);
ClientMQ GetClientMQ(pid_t clientPID, bool client = false);