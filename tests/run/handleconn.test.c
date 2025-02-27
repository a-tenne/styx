#include "handleconn.h"
#include "cleanup.h"
#include "config.util.h"
#include "errlog.h"
#include "globals.h"
#include "setup.h"
#include <criterion/criterion.h>
#include <criterion/redirect.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

static pthread_t thread;
static struct timespec spec = { .tv_sec = 0, .tv_nsec = 10000 };

static void *
disable (void *_)
{
  nanosleep (&spec, NULL);
  running = 0;
  return _;
}

TestSuite (times_out, .timeout = 2, .fini = teardown_config);

Test (_, null_check)
{
  cr_redirect_stderr ();
  handle_connection (NULL);
  cr_assert_stderr_eq_str (ERROR_MSG ("bufs cannot be NULL\n"));
}

Test (times_out, does_not_block)
{
  cr_redirect_stdout ();
  setup_config (8083);
  message_buffers *bufs = setup (2, argv);
  cr_assert_not_null (bufs);
  pthread_create (&thread, NULL, disable, NULL);
  handle_connection (bufs);
  pthread_join (thread, NULL);
  cr_assert (server == 0);
  cleanup (bufs);
}

Test (times_out, terminates_connection)
{
  cr_redirect_stdout ();
  char expect_msg[512] = { '\0' };
  setup_config (8084);
  sprintf (expect_msg, "Server running on port \033[0;32m%u\033[0m\n",
           config->port);
  strcat (expect_msg, "Connection terminated: timeout\n\nClosing server...\n");
  message_buffers *bufs = setup (2, argv);
  cr_assert_not_null (bufs);
  handle_connection (bufs);
  cr_assert (server == 0);
  cleanup (bufs);
  cr_assert_stdout_eq_str (expect_msg);
}