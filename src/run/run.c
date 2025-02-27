#include "cleanup.h"
#include "errlog.h"
#include "globals.h"
#include "handleconn.h"
#include "mem.h"
#include <aio.h>
#include <sys/select.h>
#include <time.h>
#include <unistd.h>

void
run (message_buffers *bufs)
{
  struct timespec loop_limiter = { .tv_sec = 0, .tv_nsec = 10000 };
  while (running)
    {
      fd_set read_fds;
      FD_ZERO (&read_fds);
      FD_SET (server, &read_fds);

      int ret = select (server + 1, &read_fds, NULL, NULL, &interval);
      if (ret == -1)
        {
          if (running)
            {
              free_bufs (bufs);
              EXIT_ERROR (, "select failed");
            }
          else
            break;
        }
      if (ret == 0 || !FD_ISSET (server, &read_fds))
        continue;
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
      nanosleep (&loop_limiter, NULL);
    }
}