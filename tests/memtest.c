#include "mem.h"
#include "config.h"
#include "globals.h"
#include <criterion/criterion.h>
#include <criterion/redirect.h>
#include <stdio.h>
#define ERROR_MSG(msg) ("\033[1;31mERROR:\033[0m " msg)
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

Test (buffer_creation, config_not_null, .signal = SIGSEGV)
{
  setup_buffers (NULL);
}