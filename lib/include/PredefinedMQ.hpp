#pragma once
#include "MessageTypes/RegisterMQ.hpp"
#include "MessageTypes/ClientMQ.hpp"

RegisterMQ GetRegisterMQ(pid_t cashierPID, bool cashier = false, const std::function<bool()>& errorHandler = [] { return false; });
ClientMQ GetClientMQ(pid_t clientPID, bool client = false, const std::function<bool()>& errorHandler = [] { return false; });
LoggerMQ GetLoggerMQ(pid_t loggerPID, bool logger = false, const std::function<bool()>& errorHandler = [] { return false; });