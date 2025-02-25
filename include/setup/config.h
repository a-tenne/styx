#pragma once
#include <inttypes.h>
#include <sys/time.h>

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
} *server_config;

server_config config_make (const char *file_name);
void config_destroy (server_config *config);