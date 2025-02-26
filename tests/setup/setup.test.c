#include "setup.h"
#include "cJSON.h"
#include "config.h"
#include "errlog.h"
#include "global_types.h"
#include "globals.h"
#include <criterion/criterion.h>
#include <criterion/redirect.h>
#include <sys/socket.h>
#include <unistd.h>

static server_config config = NULL;
static char file_name[100] = { '\0' };
static const char *argv[] = { "./Tests", file_name, NULL };

static void
setup_config (void)
{
  sprintf (file_name, "%d.json", getpid ());
  FILE *config_file = fopen (file_name, "w");
  cr_assert_not_null (config_file);
  cJSON *json = cJSON_CreateObject ();
  cJSON_AddNumberToObject (json, "port", 8080);
  cJSON_AddStringToObject (json, "ip", "127.0.0.1");
  cJSON_AddNumberToObject (json, "recv_header_sz", 8192);
  cJSON_AddNumberToObject (json, "recv_body_sz", 2048);
  cJSON_AddNumberToObject (json, "resp_header_sz", 16384);
  cJSON_AddNumberToObject (json, "resp_body_sz", 4194304);
  cJSON_AddNumberToObject (json, "timeout_s", 1);
  cJSON_AddNumberToObject (json, "max_clients", 10);
  char *json_text = cJSON_Print (json);
  fputs (json_text, config_file);
  free (json_text);
  cJSON_Delete (json);
  fclose (config_file);
  config = config_make (file_name);
}

static void
teardown_config (void)
{
  config_destroy (&config);
  remove (file_name);
}

Test (_, globals_initialized, .init = setup_config, .fini = teardown_config)
{
  cr_redirect_stdout ();

  message_buffers *bufs = setup (2, argv);
  cr_assert_not_null (bufs);
  fflush (stdout);
  char success_message[100] = { '\0' };
  sprintf (success_message, "Server running on port \033[0;32m%u\033[0m\n",
           config->port);
  cr_stdout_match_str (success_message);
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
  cr_stderr_match_str (ERROR_MSG ("argv cannot be NULL\n")
                           ERROR_MSG ("*argv cannot be NULL\n"));
}

Test (_, invalid_argv)
{
  cr_redirect_stderr ();
  message_buffers *bufs = setup (3, argv);
  cr_assert_null (bufs);
  cr_stderr_match_str (
      ERROR_MSG ("Invalid arguments. Usage: WebServer <opt: config_path>\n")
          ERROR_MSG ("file_path cannot be NULL\n"));
}