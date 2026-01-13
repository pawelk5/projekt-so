#pragma once

/// Creates a signal handler
/// \param sig signal number
/// \param function signal handler function
/// \throws std::runtime_error on failure
bool CreateSignalHandler(int sig, void (*function)(int));