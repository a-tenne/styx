#include "handleconn.h"
#include "errlog.h"
#include "globals.h"
#include "header.h"
#include "mem.h"
#include "request.h"
#include "response.h"
#include "state.h"
#include <stdbool.h>
#include <stdio.h>
#include <sys/select.h>
#include <unistd.h>

void
handle_connection (message_buffers *bufs)
{
  NULL_CHECK (bufs, );
  close (server);
  server = 0;
  allocate_bufs (bufs);
  connection_state state = { .code = NOT_PROCESSED,
                             .keep_alive = true,
                             .max_requests = 100,
                             .timeout = { .tv_sec = 5, .tv_usec = 0 },
                             .current_request = 100 };

  // criterion leaks memory on longer timeouts
#ifdef TEST
  state.timeout.tv_sec = 1;
#endif
  while (running && state.keep_alive && state.current_request > 0)
    {

      fd_set read_fds;
      FD_ZERO (&read_fds);
      FD_SET (connection, &read_fds);

      int ret = select (connection + 1, &read_fds, NULL, NULL, &state.timeout);
      if (ret == -1)
        {
          free_bufs (bufs);
          if (running)
            EXIT_ERROR (, "select failed");
          else
            break;
        }
      if (ret == 0 || !FD_ISSET (connection, &read_fds))
        {
          puts ("Connection terminated: timeout");
          break;
        }
      header_data *request_data = request (bufs, &state);

      if (state.code != CLOSE)
        {
          response (bufs, request_data, &state);
        }
      free_data (request_data);
      if (state.keep_alive)
        {
          --state.current_request;
          state.code = NOT_PROCESSED;
          clear_bufs (bufs);
        }
    }
}