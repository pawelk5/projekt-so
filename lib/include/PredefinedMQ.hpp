#pragma once
#include "MessageTypes/RegisterMQ.hpp"
#include "MessageTypes/ClientMQ.hpp"

/// Create or attach cash register message queue
/// \param cashierPID pid of the cashier process
/// \param cashier if true the function will create a new message queue instead of attaching existing one
/// \param errorHandler function that allows to additionally handle certain errors. the function should return false if it fails to handle any error
/// \param blocking if true sending/receiving messages will be blocking
RegisterMQ GetRegisterMQ(pid_t cashierPID, bool cashier = false, const std::function<bool()>& errorHandler = [] { return false; }, bool blocking = false);

/// Create or attach client message queue
/// \param clientPID pid of the client process
/// \param client if true the function will create a new message queue instead of attaching existing one
/// \param errorHandler function that allows to additionally handle certain errors. the function should return false if it fails to handle any error
ClientMQ GetClientMQ(pid_t clientPID, bool client = false, const std::function<bool()>& errorHandler = [] { return false; });

/// Create or attach logger message queue
/// \param loggerPID pid of the logger (main) process
/// \param logger if true the function will create a new message queue instead of attaching existing one
/// \param errorHandler function that allows to additionally handle certain errors. the function should return false if it fails to handle any error
LoggerMQ GetLoggerMQ(pid_t loggerPID, bool logger = false, const std::function<bool()>& errorHandler = [] { return false; });