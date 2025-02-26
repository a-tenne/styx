#include "run.h"
#include "cJSON.h"
#include "cleanup.h"
#include "config.h"
#include "setup.h"
#include <criterion/criterion.h>
#include <criterion/redirect.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>

static server_config config = NULL;
static char file_name[100] = { '\0' };
static const char *argv[] = { "./Tests", file_name, NULL };
static int sig = 0;
static struct timespec spec = { .tv_sec = 0, .tv_nsec = 20000 };

static void
setup_config (int port)
{
  sprintf (file_name, "%d.json", getpid ());
  FILE *config_file = fopen (file_name, "w");
  cr_assert_not_null (config_file);
  cJSON *json = cJSON_CreateObject ();
  cJSON_AddNumberToObject (json, "port", port);
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

void *
trigger_signal (void *_)
{
  nanosleep (&spec, NULL);
  raise (sig);
  return _;
}

TestSuite (signals, .timeout = 2, .fini = teardown_config);

Test (signals, sigint_breaks)
{
  setup_config (8081);
  cr_redirect_stdout ();
  pthread_t thread;
  sig = SIGINT;
  message_buffers *bufs = setup (2, argv);
  pthread_create (&thread, NULL, trigger_signal, NULL);
  run (bufs);
  cleanup (bufs);
  pthread_join (thread, NULL);
}

Test (signals, sigterm_breaks)
{
  setup_config (8082);
  cr_redirect_stdout ();
  pthread_t thread;
  sig = SIGTERM;
  message_buffers *bufs = setup (2, argv);
  pthread_create (&thread, NULL, trigger_signal, NULL);
  run (bufs);
  cleanup (bufs);
  pthread_join (thread, NULL);
}
