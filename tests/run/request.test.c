#include "request.h"
#include "cleanup.h"
#include "config.bufsizes.h"
#include "config.util.h"
#include "debug.h"
#include "errlog.h"
#include "globals.h"
#include "setup.h"
#include "state.h"
#include <arpa/inet.h>
#include <criterion/criterion.h>
#include <criterion/redirect.h>
#include <float.h>
#include <pthread.h>
#include <setup.h>
#include <time.h>
#include <unistd.h>

static pthread_t thread;
static struct sockaddr_in server_addr;
static struct timespec spec = { .tv_sec = 0, .tv_nsec = 20000 };
static file_descriptor test_client = 0;
static connection_state state = { .code = NOT_PROCESSED,
                                  .current_request = 100,
                                  .max_requests = 100,
                                  .timeout = { .tv_sec = 1, .tv_usec = 0 },
                                  .keep_alive = true };
static message_buffers *bufs = NULL;
static size_t message_length;

static void *
connect_to_server (void *msg)
{
  nanosleep (&spec, NULL);
  cr_assert (connect (test_client, (struct sockaddr *)&server_addr,
                      sizeof (server_addr))
             >= 0);
  cr_assert (send (test_client, msg, message_length, 0) >= -1);
  running = 0;
  close (test_client);
  return NULL;
}

static void
setup_connection (int port, char *msg)
{
  static const int enable = 1;
  static sockaddr_in_p server_sock;
  static socklen_t socklen = sizeof (*server_sock);
  running = 1;
  setup_config (port);
  server_sock = make_ipv4 (config);
  cr_assert ((test_client = socket (AF_INET, SOCK_STREAM, 0)) >= 0);
  cr_assert_not_null (config);
  bufs = setup_buffers (config);
  allocate_bufs (bufs);
  interval.tv_sec = config->timeout_s;
  interval.tv_usec = 0;
  cr_assert ((server = socket (AF_INET, SOCK_STREAM, 0)) >= 0);
  cr_assert (
      setsockopt (server, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof (int))
      >= 0);
  cr_assert (
      bind (server, (struct sockaddr *)server_sock, sizeof (*server_sock))
      >= 0);

  cr_assert (listen (server, config->max_clients) >= 0);
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons (config->port);
  inet_pton (AF_INET, config->addr, &server_addr.sin_addr);
  pthread_create (&thread, NULL, connect_to_server, msg);
  cr_assert (
      (connection = accept (server, (struct sockaddr *)server_sock, &socklen))
      != -1);
}
static void
teardown_connection (void)
{
  cleanup (bufs);
  pthread_join (thread, NULL);
}

Test (_, inputs_not_null, .fini = teardown_config)
{
  cr_redirect_stderr ();
  setup_config (8080);
  bufs = setup_buffers (config);
  cr_assert_not_null (bufs);
  header_data *data = request (NULL, &state);
  cr_assert_null (data);
  data = request (bufs, NULL);
  cr_assert_null (data);
  cr_assert_stderr_eq_str (ERROR_MSG ("bufs cannot be NULL\n")
                               ERROR_MSG ("state cannot be NULL\n"));
}

Test (_, recv_fail, .timeout = 1, .fini = teardown_config)
{
  cr_redirect_stdout ();
  setup_config (8085);
  bufs = setup_buffers (config);
  cr_assert_not_null (bufs);
  header_data *data = request (bufs, &state);
  cr_assert_null (data);
  cr_assert (state.code == BAD_REQUEST);
  cr_assert (!state.keep_alive);
  cr_stdout_match_str (WARN_MSG ("recv error\n"));
}

TestSuite (connecting, .fini = teardown_config, .timeout = 1);

Test (connecting, major_request_test)
{
  cr_redirect_stdout ();
  char msg[] = "GET / HTTP/1.1\r\n"
               "Content-Type: text/plain\r\n"
               "Content-Length: 11\r\n"
               "\r\nHello\0World";
  message_length = sizeof (msg);
  setup_connection (8086, msg);
  header_data *data = request (bufs, &state);
  cr_assert_str_eq (data->method, "GET");
  cr_assert_str_eq (data->path, "/");
  cr_assert_str_eq (data->version, "HTTP/1.1");
  cr_assert (data->h_vec.length == 2);
  cr_assert_str_eq ((bufs->recv.body.payload + 6), "World");
  cr_assert (state.code == NOT_PROCESSED);
  cr_assert (state.keep_alive);
  free_data (data);
  teardown_connection ();
}

Test (connecting, method_not_implemented)
{
  cr_redirect_stdout ();
  char msg[] = "PUT / HTTP/1.1\r\n"
               "Content-Type: text/plain\r\n"
               "Content-Length: 11\r\n"
               "\r\nHello\0World";
  message_length = sizeof (msg);
  setup_connection (8087, msg);
  header_data *data = request (bufs, &state);
  cr_assert_str_eq (data->method, "PUT");
  cr_assert_str_eq (data->path, "/");
  cr_assert_str_eq (data->version, "HTTP/1.1");
  cr_assert (data->h_vec.length == 2);
  cr_assert_str_eq ((bufs->recv.body.payload + 6), "World");
  cr_assert (state.code == NOT_IMPLEMENTED);
  cr_assert (state.keep_alive);
  free_data (data);
  teardown_connection ();
}

Test (connecting, empty_msg)
{
  cr_redirect_stdout ();
  char msg[] = "";
  message_length = sizeof (msg);
  setup_connection (8088, msg);
  header_data *data = request (bufs, &state);
  cr_assert_null (data);
  cr_assert (state.code == CLOSE);
  cr_assert (!state.keep_alive);
  teardown_connection ();
  cr_assert_stdout_eq_str (
      "Connection terminated: empty header\n\nClosing server...\n");
}

Test (connecting, not_http)
{
  cr_redirect_stdout ();
  char msg[] = "hello";
  message_length = sizeof (msg);
  setup_connection (8089, msg);
  header_data *data = request (bufs, &state);
  cr_assert_null (data);
  cr_assert (state.code == CLOSE);
  cr_assert (!state.keep_alive);
  teardown_connection ();
  cr_assert_stdout_eq_str (
      WARN_MSG ("Connection terminated: not HTTP\n") "\nClosing server...\n");
}

Test (connecting, header_size_exceeded)
{
  cr_redirect_stdout ();
  char msg[RECV_HEADER_SIZE];
  message_length = sizeof (msg);
  memset (msg, 'A', message_length);
  setup_connection (8090, msg);
  header_data *data = request (bufs, &state);
  cr_assert_null (data);
  cr_assert (state.code == REQU_HEAD_FIELDS_TOO_LARGE);
  cr_assert (state.keep_alive);
  char expect_msg[256] = { '\0' };
  sprintf (
      expect_msg,
      WARN_MSG ("request header size %d exceeded\n") "\nClosing server...\n",
      RECV_HEADER_SIZE);
  teardown_connection ();
  cr_assert_stdout_eq_str (expect_msg);
}
Test (connecting, content_too_large)
{
  cr_redirect_stdout ();
  char msg[256];
  sprintf (msg, "GET / HTTP/1.1\r\nContent-Length: %ld\r\n\r\n",
           (long)(RECV_BODY_SIZE + 1));
  message_length = sizeof (msg);
  setup_connection (8091, msg);
  header_data *data = request (bufs, &state);
  cr_assert_not_null (data);
  cr_assert (state.code = CONTENT_TOO_LARGE);
  cr_assert (state.keep_alive);
  free_data (data);
  teardown_connection ();
}