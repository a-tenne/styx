#include "mem.h"
#include "cJSON.h"
#include "config.h"
#include "errlog.h"
#include <arpa/inet.h>
#include <criterion/criterion.h>
#include <criterion/redirect.h>
#include <stdio.h>
#include <unistd.h>
static server_config config = NULL;
char file_name[100] = { '\0' };
static void
setup_config (void)
{
  sprintf (file_name, "%d.json", getpid ());
  FILE *config_file = fopen (file_name, "w");
  cr_assert_not_null (config_file);
  cJSON *json = cJSON_CreateObject ();
  cr_assert_not_null (json);
  cr_assert_not_null (cJSON_AddNumberToObject (json, "port", 8080));
  cr_assert_not_null (cJSON_AddStringToObject (json, "ip", "127.0.0.1"));
  cr_assert_not_null (cJSON_AddNumberToObject (json, "recv_header_sz", 8192));
  cr_assert_not_null (cJSON_AddNumberToObject (json, "recv_body_sz", 2048));
  cr_assert_not_null (cJSON_AddNumberToObject (json, "resp_header_sz", 16384));
  cr_assert_not_null (cJSON_AddNumberToObject (json, "resp_body_sz", 4194304));
  cr_assert_not_null (cJSON_AddNumberToObject (json, "timeout_s", 1));
  cr_assert_not_null (cJSON_AddNumberToObject (json, "max_clients", 10));
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

TestSuite (buffer_creation, .init = setup_config, .fini = teardown_config);

Test (_, config_not_null)
{
  cr_redirect_stderr ();
  message_buffers *bufs = setup_buffers (NULL);
  cr_assert_null (bufs);
  sockaddr_in_p ipv4 = make_ipv4 (NULL);
  cr_assert_null (ipv4);
  cr_assert_stderr_eq_str (ERROR_MSG ("config cannot be NULL\n")
                               ERROR_MSG ("config cannot be NULL\n"));
}

Test (_, buffer_not_null)
{
  cr_redirect_stderr ();
  allocate_bufs (NULL);
  free_bufs (NULL);
  clear_bufs (NULL);
  cr_assert_stderr_eq_str (ERROR_MSG ("bufs cannot be NULL\n") ERROR_MSG (
      "bufs cannot be NULL\n") ERROR_MSG ("bufs cannot be NULL\n"));
}

Test (buffer_creation, buffer_setup)
{
  message_buffers *bufs = setup_buffers (config);
  cr_assert_not_null (bufs);
  cr_assert (bufs->recv.head.size == config->recv_header_sz + 1);
  cr_assert (bufs->recv.body.size == config->recv_body_sz + 1);
  cr_assert (bufs->resp.head.size == config->resp_header_sz + 1);
  cr_assert (bufs->resp.body.size == config->resp_body_sz + 1);
  cr_assert_null (bufs->recv.head.payload);
  cr_assert_null (bufs->recv.body.payload);
  cr_assert_null (bufs->resp.head.payload);
  cr_assert_null (bufs->resp.body.payload);
  cr_assert (bufs->recv.head.bytes_written == 0);
  cr_assert (bufs->recv.body.bytes_written == 0);
  cr_assert (bufs->resp.head.bytes_written == 0);
  cr_assert (bufs->resp.body.bytes_written == 0);
}

Test (buffer_creation, buffer_alloc)
{
  message_buffers *bufs = setup_buffers (config);
  cr_assert_not_null (bufs);
  allocate_bufs (bufs);
  buffer *ptrs[4] = { &bufs->recv.head, &bufs->recv.body, &bufs->resp.head,
                      &bufs->resp.body };
  for (int i = 0; i < 4; ++i)
    {
      cr_assert_not_null (ptrs[i]->payload);
      for (char *ptr = ptrs[i]->payload;
           ptr != (ptrs[i]->payload + ptrs[i]->size); ++ptr)
        {
          cr_assert (*ptr == '\0');
        }
    }

  free_bufs (bufs);
}

Test (buffer_creation, buffer_clear)
{
  message_buffers *bufs = setup_buffers (config);
  cr_assert_not_null (bufs);
  allocate_bufs (bufs);
  buffer *ptrs[4] = { &bufs->recv.head, &bufs->recv.body, &bufs->resp.head,
                      &bufs->resp.body };
  for (int i = 0; i < 4; ++i)
    {
      cr_assert_not_null (ptrs[i]->payload);
      for (char *ptr = ptrs[i]->payload;
           ptr != (ptrs[i]->payload + ptrs[i]->size - 1); ++ptr)
        {
          *ptr = i % 2 == 0 ? 'A' : '\0';
        }
      ptrs[i]->bytes_written = ptrs[i]->size - 1;
    }
  clear_bufs (bufs);
  for (int i = 0; i < 4; ++i)
    {
      cr_assert (ptrs[i]->bytes_written == 0);
      for (char *ptr = ptrs[i]->payload;
           ptr != (ptrs[i]->payload + ptrs[i]->size); ++ptr)
        {
          cr_assert (*ptr == '\0');
        }
    }

  free_bufs (bufs);
}

Test (_, sockaddr_setup, .init = setup_config, .fini = teardown_config)
{
  sockaddr_in_p ipv4 = make_ipv4 (config);
  cr_assert_not_null (ipv4);
  cr_assert (ipv4->sin_family == AF_INET);
  cr_assert (ipv4->sin_port == htons (config->port));
  char buffer[INET_ADDRSTRLEN] = { 0 };
  cr_assert_not_null (
      inet_ntop (AF_INET, &ipv4->sin_addr.s_addr, buffer, INET_ADDRSTRLEN));
  cr_assert (strcmp (buffer, config->addr) == 0);
}
