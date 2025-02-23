#define _GNU_SOURCE
#include "mem.h"
#include "config.h"
#include "global_types.h"
#include "handle_errs.h"
#include <arpa/inet.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

sockaddr_in_p
make_ipv4 (server_config *config)
{
  static struct sockaddr_in addr;
  memset (&addr, 0, sizeof (addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons (config->port);
  int err = inet_pton (AF_INET, config->addr, &addr.sin_addr.s_addr);
  if (err == 0 || err == -1)
    {
      free (config);
      EXIT_ERROR (NULL, "Invalid ip address.");
    }
  return &addr;
}

static void *
allocate_buffer (buffer *buf)
{
  buf->payload = calloc (buf->size, sizeof (char));
  return buf->payload;
}

void
allocate_bufs (message_buffers *bufs)
{
  void *recv_head = allocate_buffer (&bufs->recv.head);
  void *recv_body = allocate_buffer (&bufs->recv.body);
  void *resp_head = allocate_buffer (&bufs->resp.head);
  void *resp_body = allocate_buffer (&bufs->resp.body);
  if (!(recv_head && recv_body && resp_head && resp_body))
    {
      free_bufs (bufs);
      EXIT_ERROR (, "Buffer allocation failed!");
    }
}

message_buffers *
setup_buffers (server_config *config)
{
  static message_buffers bufs;
  if (config == NULL)
    {
      EXIT_ERROR (NULL, "config is null");
    }
  bufs.recv.head.size = config->recv_header_sz + 1;
  bufs.recv.body.size = config->recv_body_sz + 1;
  bufs.resp.head.size = config->resp_header_sz + 1;
  bufs.resp.body.size = config->resp_body_sz + 1;
  return &bufs;
}

static void
deallocate_buffer (buffer *buf)
{
  free (buf->payload);
  buf->payload = NULL;
}

void
free_bufs (message_buffers *bufs)
{
  deallocate_buffer (&bufs->resp.head);
  deallocate_buffer (&bufs->resp.body);
  deallocate_buffer (&bufs->recv.head);
  deallocate_buffer (&bufs->recv.body);
}

void
clear_bufs (message_buffers *bufs)
{
  memset (bufs->resp.head.payload, 0, bufs->resp.head.size - 1);
  memset (bufs->resp.body.payload, 0, bufs->resp.body.size - 1);
  memset (bufs->recv.head.payload, 0, bufs->recv.head.size - 1);
  memset (bufs->recv.body.payload, 0, bufs->recv.body.size - 1);
}