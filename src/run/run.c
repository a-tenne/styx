#define _GNU_SOURCE
#include "cleanup.h"
#include "buf.h"
#include "errlog.h"
#include "globals.h"
#include "handleconn.h"
#include <sys/select.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

void
run (message_buffers *bufs)
{
  NULL_CHECK (bufs, );
  fd_set current_fds;
  FD_ZERO (&current_fds);
  FD_SET (server, &current_fds);
  while (running)
    {
      // init temp variables for select(), since it's destructive
      fd_set ready_fds = current_fds;
      struct timeval interval_copy = interval;
      // wait for connection, non blocking
      int ret = select (server + 1, &ready_fds, NULL, NULL, &interval_copy);
      if (ret == -1)
        {
          if (running)
            {
              free_bufs (bufs);
              EXIT_ERROR (, "select failed in parent process");
            }
          else
            break;
        }
      if (ret == 0 || !FD_ISSET (server, &ready_fds))
        {
          // reap child processes
          while (waitpid (-1, NULL, WNOHANG) > 0)
            ;
          continue;
        }

      connection = accept (server, (struct sockaddr *)&client_addr, &addr_len);

      if (connection == -1)
        {
          warning ("connection failed.");
          continue;
        }
      pid = fork ();
      if (pid == -1)
        {
          warning ("could not fork. Closing connection.");
          close (connection);
          continue;
        }
      if (pid == 0)
        {
          handle_connection (bufs);
          // this is normally executed in main
#ifdef TEST
          cleanup (bufs);
#endif
          return;
        }
      else
        {
          close (connection);
          connection = 0;
        }
    }
}
