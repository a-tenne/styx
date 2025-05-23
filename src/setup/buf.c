#define _GNU_SOURCE
#include "buf.h"
#include "config.h"
#include "errlog.h"
#include <arpa/inet.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

static void *
allocate_buffer (buffer *buf)
{
#if defined(DEBUG) || defined(TEST)
  if (buf->size <= 0)
    return NULL;
#endif
  buf->payload = calloc (buf->size, sizeof (char));
  return buf->payload;
}

void
allocate_bufs (message_buffers *bufs)
{
  NULL_CHECK (bufs, );
  void *recv_head = allocate_buffer (&bufs->recv.head);
  void *recv_body = allocate_buffer (&bufs->recv.body);
  void *resp_head = allocate_buffer (&bufs->resp.head);
  void *resp_body = allocate_buffer (&bufs->resp.body);
  if (!(recv_head && recv_body && resp_head && resp_body))
    {
      free_bufs (bufs);
      EXIT_ERROR (, "buffer allocation failed");
    }
}

message_buffers *
setup_buffers (server_config config)
{
  NULL_CHECK (config, NULL);
  static message_buffers bufs;
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
  NULL_CHECK (bufs, );
  deallocate_buffer (&bufs->resp.head);
  deallocate_buffer (&bufs->resp.body);
  deallocate_buffer (&bufs->recv.head);
  deallocate_buffer (&bufs->recv.body);
}

static void
clear_buffer (buffer *buf)
{
  NULL_CHECK (buf, );
  memset (buf->payload, 0, buf->size - 1);
  buf->bytes_written = 0;
}
void
clear_bufs (message_buffers *bufs)
{
  NULL_CHECK (bufs, );
  clear_buffer (&bufs->resp.head);
  clear_buffer (&bufs->resp.body);
  clear_buffer (&bufs->recv.head);
  clear_buffer (&bufs->recv.body);
}
