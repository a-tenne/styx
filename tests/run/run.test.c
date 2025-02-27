#include "run.h"
#include "cJSON.h"
#include "cleanup.h"
#include "config.h"
#include "config.util.h"
#include "setup.h"
#include <criterion/criterion.h>
#include <criterion/redirect.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>

static int sig = 0;
static struct timespec spec = { .tv_sec = 0, .tv_nsec = 20000 };

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
  cr_assert_not_null (bufs);
  pthread_create (&thread, NULL, trigger_signal, NULL);
  run (bufs);
  pthread_join (thread, NULL);
}

Test (signals, sigterm_breaks)
{
  setup_config (8082);
  cr_redirect_stdout ();
  pthread_t thread;
  sig = SIGTERM;
  message_buffers *bufs = setup (2, argv);
  cr_assert_not_null (bufs);
  pthread_create (&thread, NULL, trigger_signal, NULL);
  run (bufs);
  pthread_join (thread, NULL);
}
