#include "errlog.h"
#include "globals.h"
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

void
exit_error (const char *error_msg, ...)
{
  va_list list;
  va_start (list, error_msg);
#ifndef TEST
  perror (NULL);
  errno = 0;
#endif
  fprintf (stderr, ERROR_MSG ());
  vfprintf (stderr, error_msg, list);
  putc ('\n', stderr);
  va_end (list);
  if (server != -1 && server != 0)
    close (server);
  if (connection != -1 && connection != 0)
    close (connection);
  fflush (stderr);
#ifndef TEST
  if (pid != 0)
    printf ("Closing Server...\n");
  fflush (stdout);
  exit (1);
#endif
}

void
warning (const char *warning_msg, ...)
{
  va_list list;
  va_start (list, warning_msg);
#ifndef TEST
  if (errno)
    errno = 0;
#endif
  printf (WARN_MSG ());
  vprintf (warning_msg, list);
  putchar ('\n');
  va_end (list);
  fflush (stdout);
}
