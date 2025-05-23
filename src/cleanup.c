#include "buf.h"
#include "errlog.h"
#include "globals.h"
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

void
cleanup (message_buffers *bufs)
{
  NULL_CHECK (bufs, );
  if (pid != 0)
    {
      close (server);
#ifdef TEST
      server = 0;
#endif
      wait (NULL);
      puts ("\nClosing server...");
    }
  else
    {
      close (connection);
#ifdef TEST
      connection = 0;
#endif
      free_bufs (bufs);
    }
  fflush (NULL);
}
