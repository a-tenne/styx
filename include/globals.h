#pragma once
#include "global_types.h"
#include <arpa/inet.h>
#include <signal.h>

/**
 * @brief Main loop run condition
 *
 * Set to 1 to run, 0 to stop.
 * @note Important for clean up
 */
extern volatile sig_atomic_t running;
/**
 * @brief Global file descriptor for the listener socket
 */
extern file_descriptor server;
/**
 * @brief Global file descriptor for the connection socket
 */
extern file_descriptor connection;

/**
 * @brief Global interval for server time outs.
 */
extern struct timeval interval;
extern struct sockaddr_in client_addr;
extern socklen_t addr_len;
extern pid_t pid;