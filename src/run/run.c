#include "globals.h"
#include "handle_errs.h"
#include "handleconn.h"
#include "mem.h"
#include <sys/select.h>
#include <unistd.h>

#include <stdio.h>

void
run (message_buffers *bufs)
{
  while (running)
    {
      fd_set read_fds;
      FD_ZERO (&read_fds);
      FD_SET (server, &read_fds);
      // 1 for 1 second interval, 0 for 0 (additional) nanoseconds;
      int ret = select (server + 1, &read_fds, NULL, NULL, &interval);
      if (ret == -1)
        {
          if (running)
            {
              free_bufs (bufs);
              exit_error ("select failed");
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
          return;
        }
      else
        {
          close (connection);
          connection = 0;
        }
    }
}