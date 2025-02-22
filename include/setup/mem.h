#pragma once
#include "config.h"
#include "types.h"

typedef struct
{
  char *payload;
  long size;
} buffer;

typedef struct
{
  buffer head;
  buffer body;
} message;

typedef struct
{
  message recv;
  message resp;
} message_buffers;

/**
 * @brief Creates and returns a pointer to a ipv4 socket address struct.
 * @param ip IPv4 Address in dotted notation.
 * @param port The port to be used.
 * @return A pointer to a singleton socket address struct.
 * Initializes a singleton, zeroes it and sets all values, then returns a
 * pointer to it.
 */
sockaddr_in_p make_ipv4 (server_config *config);

message_buffers *setup_buffers (server_config *config);
void free_bufs (message_buffers *bufs);
void allocate_bufs (message_buffers *bufs);
void clear_bufs (message_buffers *bufs);