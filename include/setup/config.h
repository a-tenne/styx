#pragma once
#include "handle_errs.h"
#include <inttypes.h>
#include <sys/time.h>
#define PORT (1 << 0)
#define ADDR (1 << 1)
#define RECV_HEAD (1 << 2)
#define RECV_BODY (1 << 3)
#define RESP_HEAD (1 << 4)
#define RESP_BODY (1 << 5)
#define TIMEOUT (1 << 6)
#define CLIENTS (1 << 7)
#define IS_DUP(flags, mask) (flags & mask)
#define SET_FLAG(flags, mask) (flags |= mask)

typedef struct
{
  uint16_t port;
  char addr[16];
  long recv_header_sz;
  long recv_body_sz;
  long resp_header_sz;
  long resp_body_sz;
  time_t timeout_s;
  int max_clients;
} server_config;

server_config *load_config (const char *file_name);