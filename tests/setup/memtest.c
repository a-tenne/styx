#include "mem.h"
#include "config.h"
#include "errlog.h"
#include "globals.h"
#include <criterion/criterion.h>
#include <criterion/redirect.h>
#include <stdio.h>
server_config config = NULL;

static void
setup (void)
{
  config = config_make ("serverconfig.json");
}

static void
teardown (void)
{
  config_destroy (&config);
}

TestSuite (buffer_creation, .init = setup, .fini = teardown);

Test (buffer_creation, config_not_null)
{
  cr_redirect_stderr ();
  message_buffers *bufs = setup_buffers (NULL);
  cr_assert_null (bufs);
  cr_assert_stderr_eq_str (ERROR_MSG ("config cannot be NULL\n"));
}