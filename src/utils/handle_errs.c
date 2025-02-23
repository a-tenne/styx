#include "handle_errs.h"
#include "global_types.h"
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
  fprintf (stderr, "\033[1;31mERROR:\033[0m ");
  vfprintf (stderr, error_msg, list);
  fputc ('\n', stderr);
  va_end (list);
  if (server != -1 && server != 0)
    close (server);
  if (connection != -1 && connection != 0)
    close (connection);
#ifndef TEST
  printf ("Closing Server...\n");
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
  printf ("\033[1;35mWARNING:\033[0m ");
  vprintf (warning_msg, list);
  putchar ('\n');
  va_end (list);
}