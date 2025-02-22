#include "globals.h"
#include "mem.h"
#include <stdio.h>
#include <unistd.h>

void
cleanup (message_buffers *bufs)
{
  if (pid != 0)
    {
      close (server);
      printf ("\nClosing server...\n");
    }
  else
    {
      close (connection);
      free_bufs (bufs);
    }
}