#ifdef DEBUG
#include "debug.h"
#endif
#define _GNU_SOURCE
#include "globals.h"
#include "request.h"
#include "state.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static size_t
process_header_fields (header_data *data, connection_state *state)
{
  size_t content_length = 0;
  char *saveptr;
  char *token;
  for (header temp = data->header_list.head; temp != NULL; temp = temp->next)
    {
      if (!strcmp ("Content-Length", temp->key))
        {
          content_length = strtoul (temp->value, NULL, 10);
        }
      else if (!strcmp ("Connection", temp->key))
        {
          token = strtok_r (temp->value, ", ", &saveptr);
          while (token)
            {
              if (!strcasecmp (token, "close"))
                state->keep_alive = false;
              token = strtok_r (NULL, ", ", &saveptr);
            }
        }
    }

  return content_length;
}

header_data *
request (message_buffers *bufs, connection_state *state)
{
  ssize_t bytes_read = 0;
  char *header_end = NULL;
  allocate_bufs (bufs);
  bytes_read = recv (connection, bufs->recv.head.payload,
                     bufs->recv.head.size - 1, 0);

  if (bytes_read == -1)
    {
      warning ("recv error");
      state->code = BAD_REQUEST;
      state->keep_alive = false;
      return NULL;
    }
  if (bufs->recv.head.payload == NULL || *bufs->recv.head.payload == '\0')
    {
      puts ("Connection terminated: empty header");
      state->code = CLOSE;
      state->keep_alive = false;
      return NULL;
    }
  header_end = strstr (bufs->recv.head.payload, "\r\n\r\n");
  if (header_end == NULL)
    {
      warning ("request header size %ld exceeded", bufs->recv.head.size);
      state->code = REQU_HEAD_FIELDS_TOO_LARGE;
      return NULL;
    }
  char *body_start = header_end + 4;
  size_t body_len = 0;
  for (char *ptr = body_start; *ptr; ++ptr)
    ++body_len;
  memmove (bufs->recv.body.payload, body_start, body_len);
  *body_start = '\0';
  header_data *data = parse (bufs->recv.head.payload);

  if (data == NULL)
    {
      state->code = BAD_REQUEST;
      return data;
    }
  if (strcmp ("GET", data->method) && strcmp ("HEAD", data->method))
    {
      state->code = NOT_IMPLEMENTED;
      return data;
    }
  size_t content_length = process_header_fields (data, state);

  if (content_length > (size_t)(bufs->recv.body.size - 1))
    {
      state->code = CONTENT_TOO_LARGE;
      return data;
    }
  else if (content_length > body_len)
    {
      bytes_read = recv (connection, (bufs->recv.body.payload + body_len),
                         content_length - body_len, 0);
      if (bytes_read == -1)
        {
          state->code = INTERNAL_SERVER_ERROR;
          warning ("recv error with body");
          return data;
        }
    }
  else
    {
      bufs->recv.body.payload[content_length] = '\0';
    }
#ifdef DEBUG
  print_data (data);
  printf ("Body:");
  if (*bufs->recv.body.payload)
    printf ("\n%s\n", bufs->recv.body.payload);
  else
    puts (" empty body");
#endif
  return data;
}