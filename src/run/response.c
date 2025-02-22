#include "response.h"
#include "globals.h"
#include "state.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#define HEADER(bufs) (bufs->resp.head)
#define BODY(bufs) (bufs->resp.body)
#define FORMAT_HEADER(http_code, msg)                                         \
  ("HTTP/1.1 " #http_code " " msg "\r\n\r\n")
#define SEND(buf, result, length)                                             \
  do                                                                          \
    {                                                                         \
      length = strlen (buf);                                                  \
      result = send (connection, buf, length * sizeof (char), 0);             \
      if (result != length)                                                   \
        return false;                                                         \
    }                                                                         \
  while (0)

static bool
send_response (message *response)
{
  ssize_t res = 0;
  ssize_t len = 0;
  SEND (response->head.payload, res, len);
  SEND (response->body.payload, res, len);
  return true;
}

static bool
copy_into_buffer (buffer *buff, const char *msg)
{
  if ((size_t)buff->size >= strlen (msg))
    {
      warning ("message too big for buffer");
      return false;
    }
  strcpy (buff->payload, msg);
  return true;
}

static void
empty_buffer (buffer *buff)
{
  *buff->payload = '\0';
}

static bool
get_file (buffer *body, const char *path)
{
  FILE *fp = fopen (path, "r");
  if (fp == NULL)
    {
      warning ("cannot open %s", path);
      return false;
    }
  if (fseek (fp, 0, SEEK_END) == -1)
    {
      fclose (fp);
      warning ("cannot find end of %s", path);
      return false;
    }
  long file_size = ftell (fp);
  if (file_size <= 0)
    {
      fclose (fp);
      warning ("cannot read %s", path);
      return false;
    }
  if (file_size >= body->size)
    {
      fclose (fp);
      warning ("%s too big to be sent", path);
      return false;
    }
  rewind (fp);
  size_t bytes_read = fread (body->payload, file_size, 1, fp);
  fclose (fp);
  if (bytes_read != 1)
    {
      warning ("%s cannot be read correctly", path);
      return false;
    }
  return true;
}

void
response (message_buffers *bufs, header_data *request_data,
          connection_state *state)
{
  bool result = false;
  if (state->code == NOT_PROCESSED)
    {
    }
  switch (state->code)
    {

    case CONTENT_TOO_LARGE:
      break;
    case REQU_HEAD_FIELDS_TOO_LARGE:
      break;
    case NOT_IMPLEMENTED:
      break;
    case NOT_FOUND:
      break;
    case BAD_REQUEST:
      break;
    case OK:
      break;
    case INTERNAL_SERVER_ERROR:
    default:
      result = copy_into_buffer (&HEADER (bufs),
                                 FORMAT_HEADER (500, "Internal Server Error"));
      if (!result)
        return;
      empty_buffer (&BODY (bufs));
#ifdef DEBUG
      if (state->code != INTERNAL_SERVER_ERROR)
        warning ("invalid status state->code %d", state->code);
#endif
    }
}