#include "config.h"
#include "errlog.h"
#include <criterion/criterion.h>
#include <criterion/redirect.h>
#include <signal.h>
static void
setup ()
{
}

static void
teardown ()
{
}

TestSuite (parse_file, .init = setup, .fini = teardown);

Test (config, not_null)
{
  cr_redirect_stderr ();
  server_config config = config_make (NULL);
  cr_assert_null (config);
  cr_stderr_match_str (ERROR_MSG ("file_name cannot be NULL\n"));
}