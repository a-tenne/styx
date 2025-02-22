#include "setup.h"
#include "mem.h"
#include <arpa/inet.h>
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#define TCP 0
#define DEFAULT_PATH "serverconfig.json"

file_descriptor server = 0, connection = 0;
struct timeval interval;
volatile sig_atomic_t running = 1;
struct sockaddr_in client_addr;
socklen_t addr_len = sizeof (struct sockaddr_in);
pid_t pid;

static const char *
handle_args (int argc, char const **argv)
{
  if (argc > 2)
    exit_error ("Invalid arguments. Usage: WebServer <opt: config_path>");
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

static void
static_exists ()
{
  struct stat info;
  if (!stat ("static", &info) && S_ISDIR (info.st_mode))
    return;
  exit_error ("please provide a /static directory.");
}

message_buffers *
setup (int argc, char const **argv)
{
  pid = getpid ();
  const char *file_path = handle_args (argc, argv);
  static_exists ();
  server_config *config = load_config (file_path ? file_path : DEFAULT_PATH);
  //  getting the fd from the socket() syscall using IPv4 and TCP
  server = socket (AF_INET, SOCK_STREAM, TCP);
  if (server < 0)
    {
      exit_error ("server socket error");
    }
  const int enable = 1;
  if (setsockopt (server, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof (int)) < 0)
    {
      exit_error ("cannot set socket options");
    }
  // set server struct
  sockaddr_in_p addr = make_ipv4 (config);
  // initialize client struct
  memset (&client_addr, 0, sizeof (client_addr));
  // bind server to socket and listen
  if (bind (server, (struct sockaddr *)addr, sizeof (*addr)) < 0)
    {
      free (config);
      exit_error ("cannot bind to port %u", ntohs (addr->sin_port));
    }
  if (listen (server, config->max_clients) == -1)
    {
      free (config);
      exit_error ("cannot listen to port %u", ntohs (addr->sin_port));
    }
  message_buffers *bufs = setup_buffers (config);
  interval.tv_sec = config->timeout_s;
  interval.tv_usec = 0;
  // Start interrupt handlers
  signal (SIGTERM, signal_handler);
  signal (SIGINT, signal_handler);
  printf ("Server running on port \033[0;32m%u\033[0m\n",
          ntohs (addr->sin_port));
  free (config);
  return bufs;
}