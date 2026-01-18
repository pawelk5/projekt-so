#pragma once
#include "MessageTypes/RegisterMQ.hpp"
#include "MessageTypes/ClientMQ.hpp"
#include "MessageTypes/AttractionMQ.hpp"
#include "MessageTypes/RestaurantMQ.hpp"

/// Create or attach cash register message queue
/// \param cashierPID pid of the cashier process
/// \param cashier if true the function will create a new message queue instead of attaching existing one
/// \param errorHandler function that allows to additionally handle certain errors. the function should return false if it fails to handle any error
/// \param blocking if true sending/receiving messages will be blocking
RegisterMQ GetRegisterMQ(pid_t cashierPID, bool cashier = false, const std::function<bool()>& errorHandler = [] { return false; }, bool blocking = false);

/// Create or attach client message queue (for client-park communication)
/// \param clientPID pid of the client process
/// \param client if true the function will create a new message queue instead of attaching existing one
/// \param errorHandler function that allows to additionally handle certain errors. the function should return false if it fails to handle any error
ClientMQ GetClientParkMQ(pid_t clientPID, bool client = false, const std::function<bool()>& errorHandler = [] { return false; });

/// Create or attach client message queue (for client-attraction communication)
/// \param clientPID pid of the client process
/// \param client if true the function will create a new message queue instead of attaching existing one
/// \param attractionID attraction id
/// \param errorHandler function that allows to additionally handle certain errors. the function should return false if it fails to handle any error
ClientMQ GetClientAttractionMQ(pid_t clientPID, short attractionID, bool client = false, const std::function<bool()>& errorHandler = [] { return false; });

/// Create or attach client message queue (for client-restaurant communication)
/// \param clientPID pid of the client process
/// \param client if true the function will create a new message queue instead of attaching existing one
/// \param errorHandler function that allows to additionally handle certain errors. the function should return false if it fails to handle any error
ClientMQ GetClientRestaurantMQ(pid_t clientPID, bool client = false, const std::function<bool()>& errorHandler = [] { return false; });

/// Create or attach logger message queue
/// \param loggerPID pid of the logger (main) process
/// \param logger if true the function will create a new message queue instead of attaching existing one
/// \param errorHandler function that allows to additionally handle certain errors. the function should return false if it fails to handle any error
LoggerMQ GetLoggerMQ(pid_t loggerPID, bool logger = false, const std::function<bool()>& errorHandler = [] { return false; });

/// Create or attach attraction message queue
/// \param attractionPID pid of the attraction process
/// \param worker if true the function will create a new message queue instead of attaching existing one
/// \param errorHandler function that allows to additionally handle certain errors. the function should return false if it fails to handle any error
AttractionMQ GetAttractionMQ(pid_t attractionPID, bool worker = false, const std::function<bool()>& errorHandler = [] { return false; }, bool blocking = false);

// Create or attach attraction message queue
/// \param restaurantPID pid of the restaurant process
/// \param worker if true the function will create a new message queue instead of attaching existing one
/// \param errorHandler function that allows to additionally handle certain errors. the function should return false if it fails to handle any error
RestaurantMQ GetRestaurantMQ(pid_t restaurantPID, bool worker = false, const std::function<bool()>& errorHandler = [] { return false; }, bool blocking = false);
