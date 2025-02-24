#include "globals.h"
#include "mem.h"
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

void
cleanup (message_buffers *bufs)
{
  int status;
  if (pid != 0)
    {
      close (server);
      wait (&status);
      printf ("\nClosing server...\n");
    }
  else
    {
      close (connection);
      free_bufs (bufs);
    }
  fflush (NULL);
}