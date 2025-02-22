#include "handleconn.h"
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
  close (server);
  server = 0;
  allocate_bufs (bufs);
  connection_state state = { .code = NOT_PROCESSED,
                             .keep_alive = true,
                             .max_requests = 100,
                             .timeout = { .tv_sec = 5, .tv_usec = 0 } };
  int current_requests = state.max_requests;
  while (state.keep_alive && current_requests > 0)
    {
      fd_set read_fds;
      FD_ZERO (&read_fds);
      FD_SET (connection, &read_fds);
      // 1 for 1 second interval, 0 for 0 (additional) nanoseconds;
      int ret = select (connection + 1, &read_fds, NULL, NULL, &state.timeout);
      if (ret == -1)
        {
          free_bufs (bufs);
          exit_error ("select() failed");
        }
      if (ret == 0 || !FD_ISSET (connection, &read_fds))
        {
          puts ("Connection terminated: timeout");
        }
      header_data *request_data = request (bufs, &state);
      if (state.code != CLOSE)
        {
          // response (bufs, request_data, &state);
        }
      free_data (request_data);
      if (state.keep_alive)
        {
          --current_requests;
          clear_bufs (bufs);
        }
    }
}