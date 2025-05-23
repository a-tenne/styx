#define _GNU_SOURCE
#include "setup.h"
#include "buf.h"
#include "errlog.h"
#include <arpa/inet.h>
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#ifndef TEST
#define STATIC "static"

#else
#define STATIC "tests/static"
#endif

#define TCP 0
#define DEFAULT_PATH "serverconfig.json"

// initialize globals
file_descriptor server = 0, connection = 0;
struct timeval interval;
volatile sig_atomic_t running = 1;
struct sockaddr_in client_addr = { 0 };
socklen_t addr_len = sizeof (struct sockaddr_in);
pid_t pid = -1;

static const char *
handle_args (int argc, char const **argv)
{
  if (argc > 2)
    EXIT_ERROR (NULL,
                "Invalid arguments. Usage: WebServer <opt: config_path>");
  if (argc == 2)
    return argv[1];
  return NULL;
}

static void
signal_handler (int signal)
{
  if (signal == SIGINT || signal == SIGTERM)
    running = 0;
}

static bool
static_exists ()
{
  struct stat info;
  return stat (STATIC, &info) == 0 && S_ISDIR (info.st_mode);
}

sockaddr_in_p
make_ipv4 (server_config config)
{
  NULL_CHECK (config, NULL);
  static struct sockaddr_in addr;
  memset (&addr, 0, sizeof (addr));
  addr.sin_family = AF_INET;
  // correct port notation
  addr.sin_port = htons (config->port);
  // correct ip notation
  int err = inet_pton (AF_INET, config->addr, &addr.sin_addr.s_addr);
  if (err == 0 || err == -1)
    {
      config_destroy (&config);
      EXIT_ERROR (NULL, "invalid ip address");
    }
  return &addr;
}

message_buffers *
setup (int argc, char const **argv)
{
  static struct timeval timeout = { .tv_sec = 1 };
  NULL_CHECK (argv, NULL);
  NULL_CHECK (*argv, NULL);
  pid = getpid ();
  const char *file_path = handle_args (argc, argv);
#ifdef TEST
  NULL_CHECK (file_path, NULL);
#endif
  if (!static_exists ())
    EXIT_ERROR (NULL, "please provide a /static directory.");

  server_config config
      = config_make (file_path != NULL ? file_path : DEFAULT_PATH);
  NULL_CHECK (config, NULL);
  //  getting the fd from the socket() syscall using IPv4 and TCP
  server = socket (AF_INET, SOCK_STREAM, TCP);
  if (server < 0)
    {
      EXIT_ERROR (NULL, "server socket error");
    }
  const int enable = 1;
  if (setsockopt (server, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof (int)) < 0)
    {
#ifdef DEBUG
      exit_error ("cannot set socket options");
#endif
    }
  if (setsockopt (server, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof (timeout))
      < 0)
    {
#ifdef DEBUG
      exit_error ("cannot set socket options");
#endif
    }
  // init server address struct
  sockaddr_in_p addr = make_ipv4 (config);
  NULL_CHECK (addr, NULL);
  // bind server to socket and listen
  if (bind (server, (struct sockaddr *)addr, sizeof (*addr)) < 0)
    {
      config_destroy (&config);
      EXIT_ERROR (NULL, "cannot bind to port %u", ntohs (addr->sin_port));
    }
  if (listen (server, config->max_clients) == -1)
    {
      config_destroy (&config);
      EXIT_ERROR (NULL, "cannot listen to port %u", ntohs (addr->sin_port));
    }
  message_buffers *bufs = setup_buffers (config);
  interval.tv_sec = config->timeout_s;
  interval.tv_usec = 0;
  // Start interrupt handlers
  signal (SIGTERM, signal_handler);
  signal (SIGINT, signal_handler);
  printf ("Server running on port \033[0;32m%u\033[0m\n",
          ntohs (addr->sin_port));
  config_destroy (&config);
  return bufs;
}
