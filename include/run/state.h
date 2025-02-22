#pragma once
#include <stdbool.h>
#include <sys/types.h>
typedef enum
{
  CLOSE = -2,
  NOT_PROCESSED = -1,
  OK = 200,
  BAD_REQUEST = 400,
  NOT_FOUND = 404,
  CONTENT_TOO_LARGE = 413,
  UNSUPPORTED_MEDIA_TYPE = 415,
  EXPECTATION_FAILED = 417,
  REQU_HEAD_FIELDS_TOO_LARGE = 431,
  INTERNAL_SERVER_ERROR = 500,
  NOT_IMPLEMENTED = 501,
} status;
typedef struct
{
  struct timeval timeout;
  bool keep_alive;
  int max_requests;
  status code;
} connection_state;