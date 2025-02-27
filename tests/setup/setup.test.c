#include "setup.h"
#include "cJSON.h"
#include "config.h"
#include "config.util.h"
#include "errlog.h"
#include "global_types.h"
#include "globals.h"
#include <criterion/criterion.h>
#include <criterion/redirect.h>
#include <sys/socket.h>
#include <unistd.h>

static void
_setup (void)
{
  setup_config (8080);
}

static void
teardown (void)
{
  teardown_config ();
}

Test (_, globals_initialized, .init = _setup, .fini = teardown)
{
  cr_redirect_stdout ();
  message_buffers *bufs = setup (2, argv);
  cr_assert_not_null (bufs);
  fflush (stdout);
  char success_message[100] = { '\0' };
  sprintf (success_message, "Server running on port \033[0;32m%u\033[0m\n",
           config->port);
  cr_assert_stdout_eq_str (success_message);
  cr_assert (server != 0);
  cr_assert (connection == 0);
  cr_assert (interval.tv_sec != 0);
  cr_assert (addr_len == sizeof (struct sockaddr_in));
  cr_assert (pid != -1);
  close (server);
}

Test (_, null_argv)
{
  char const *nil = NULL;
  cr_redirect_stderr ();
  message_buffers *bufs = setup (1, NULL);
  cr_assert_null (bufs);
  bufs = setup (1, &nil);
  cr_assert_null (bufs);
  cr_assert_stderr_eq_str (ERROR_MSG ("argv cannot be NULL\n")
                               ERROR_MSG ("*argv cannot be NULL\n"));
}

Test (_, invalid_argv)
{
  cr_redirect_stderr ();
  message_buffers *bufs = setup (3, argv);
  cr_assert_null (bufs);
  cr_assert_stderr_eq_str (
      ERROR_MSG ("Invalid arguments. Usage: WebServer <opt: config_path>\n")
          ERROR_MSG ("file_path cannot be NULL\n"));
}