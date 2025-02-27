#include "config.h"
#include "cJSON.h"
#include "errlog.h"
#include <criterion/criterion.h>
#include <criterion/redirect.h>
#include <limits.h>
#include <signal.h>
#include <unistd.h>
static char file_name[100] = { '\0' };
#define JSON_FORMAT                                                           \
  "{\n"                                                                       \
  "\t\"port\": <number>,\n"                                                   \
  "\t\"ip\": <string>,\n"                                                     \
  "\t\"recv_header_sz\": <number>,\n"                                         \
  "\t\"recv_body_sz\": <number>,\n"                                           \
  "\t\"resp_header_sz\": <number>,\n"                                         \
  "\t\"resp_body_sz\": <number>,\n"                                           \
  "\t\"timeout_s\": <number>,\n"                                              \
  "\t\"max_clients\": <number>\n"                                             \
  "}\n"

static void
setup (char *(*create_json_str) ())
{
  sprintf (file_name, "%d.json", getpid ());
  FILE *config_file = fopen (file_name, "w");
  cr_assert_not_null (config_file);
  char *json_text = create_json_str ();
  fputs (json_text, config_file);
  free (json_text);
  fclose (config_file);
}

static void
teardown ()
{
  remove (file_name);
}

TestSuite (parse_file, .fini = teardown);

Test (_, not_null)
{
  cr_redirect_stderr ();
  server_config config = config_make (NULL);
  cr_assert_null (config);
  cr_assert_stderr_eq_str (ERROR_MSG ("file_name cannot be NULL\n"));
}

static char *
missing_ip ()
{
  cJSON *json = cJSON_CreateObject ();
  cJSON_AddNumberToObject (json, "port", 8080);
  cJSON_AddNumberToObject (json, "recv_header_sz", 8192);
  cJSON_AddNumberToObject (json, "recv_body_sz", 2048);
  cJSON_AddNumberToObject (json, "resp_header_sz", 16384);
  cJSON_AddNumberToObject (json, "resp_body_sz", 4194304);
  cJSON_AddNumberToObject (json, "timeout_s", 1);
  cJSON_AddNumberToObject (json, "max_clients", 10);
  char *json_text = cJSON_Print (json);
  cJSON_Delete (json);
  return json_text;
}

Test (parse_file, attribute_missing)
{
  cr_redirect_stderr ();
  setup (missing_ip);
  cr_expect_null (config_make (file_name));
  cr_assert_stderr_eq_str (
      ERROR_MSG ("json incomplete. Expected format: " JSON_FORMAT));
}

static char *
no_attributes ()
{
  cJSON *json = cJSON_CreateObject ();
  char *json_text = cJSON_Print (json);
  cJSON_Delete (json);
  return json_text;
}

Test (parse_file, empty_object)
{
  cr_redirect_stderr ();
  setup (no_attributes);
  cr_expect_null (config_make (file_name));
  cr_assert_stderr_eq_str (
      ERROR_MSG ("json incomplete. Expected format: " JSON_FORMAT));
}

static char *
unexpected_attribute ()
{
  cJSON *json = cJSON_CreateObject ();
  cJSON_AddNumberToObject (json, "port", 8080);
  cJSON_AddNumberToObject (json, "recv_header_sz", 8192);
  cJSON_AddNumberToObject (json, "recv_body_sz", 2048);
  cJSON_AddNumberToObject (json, "resp_header_sz", 16384);
  cJSON_AddNumberToObject (json, "resp_body_sz", 4194304);
  cJSON_AddNumberToObject (json, "timeout_s", 1);
  cJSON_AddFalseToObject (json, "bool");
  cJSON_AddNumberToObject (json, "max_clients", 10);
  char *json_text = cJSON_Print (json);
  cJSON_Delete (json);
  return json_text;
}

Test (parse_file, invalid_attribute)
{
  cr_redirect_stderr ();
  setup (unexpected_attribute);
  cr_expect_null (config_make (file_name));
  cr_assert_stderr_eq_str (
      ERROR_MSG ("invalid attribute in json. Expected format: ") JSON_FORMAT);
}

static char *
two_ports ()
{
  cJSON *json = cJSON_CreateObject ();
  cJSON_AddNumberToObject (json, "port", 8080);
  cJSON_AddNumberToObject (json, "port", 8081);
  cJSON_AddStringToObject (json, "ip", "127.0.0.1");
  cJSON_AddNumberToObject (json, "recv_header_sz", 8192);
  cJSON_AddNumberToObject (json, "recv_body_sz", 2048);
  cJSON_AddNumberToObject (json, "resp_header_sz", 16384);
  cJSON_AddNumberToObject (json, "resp_body_sz", 4194304);
  cJSON_AddNumberToObject (json, "timeout_s", 1);
  cJSON_AddNumberToObject (json, "max_clients", 10);
  char *json_text = cJSON_Print (json);
  cJSON_Delete (json);
  return json_text;
}

Test (parse_file, attribute_twice)
{
  cr_redirect_stdout ();
  setup (two_ports);
  server_config config = config_make (file_name);
  cr_expect_not_null (config);
  cr_assert (config->port == 8081);
  cr_assert_stdout_eq_str (WARN_MSG ("port set twice\n"));
  config_destroy (&config);
  cr_assert_null (config);
}

static char *
invalid_port ()
{
  cJSON *json = cJSON_CreateObject ();
  cJSON_AddNumberToObject (json, "port", -12);
  cJSON_AddStringToObject (json, "ip", "127.0.0.1");
  cJSON_AddNumberToObject (json, "recv_header_sz", 8192);
  cJSON_AddNumberToObject (json, "recv_body_sz", 2048);
  cJSON_AddNumberToObject (json, "resp_header_sz", 16384);
  cJSON_AddNumberToObject (json, "resp_body_sz", 4194304);
  cJSON_AddNumberToObject (json, "timeout_s", 1);
  cJSON_AddNumberToObject (json, "max_clients", 10);
  char *json_text = cJSON_Print (json);
  cJSON_Delete (json);
  return json_text;
}

Test (parse_file, negative_port_number)
{
  cr_redirect_stderr ();
  setup (invalid_port);
  server_config config = config_make (file_name);
  cr_assert_null (config);
  cr_assert_stderr_eq_str (ERROR_MSG ("invalid port number: -12\n"));
}

static char *
max_clients_exceeded ()
{
  cJSON *json = cJSON_CreateObject ();
  cJSON_AddNumberToObject (json, "port", 8080);
  cJSON_AddStringToObject (json, "ip", "127.0.0.1");
  cJSON_AddNumberToObject (json, "recv_header_sz", 8192);
  cJSON_AddNumberToObject (json, "recv_body_sz", 2048);
  cJSON_AddNumberToObject (json, "resp_header_sz", 16384);
  cJSON_AddNumberToObject (json, "resp_body_sz", 4194304);
  cJSON_AddNumberToObject (json, "timeout_s", 1);
  cJSON_AddNumberToObject (json, "max_clients", 200);
  char *json_text = cJSON_Print (json);
  cJSON_Delete (json);
  return json_text;
}

Test (parse_file, too_many_clients)
{
  cr_redirect_stderr ();
  setup (max_clients_exceeded);
  server_config config = config_make (file_name);
  cr_assert_null (config);
  cr_assert_stderr_eq_str (
      ERROR_MSG ("maximum value for max_clients(100) exceeded\n"));
}

static char *
zero_clients ()
{
  cJSON *json = cJSON_CreateObject ();
  cJSON_AddNumberToObject (json, "port", 8080);
  cJSON_AddStringToObject (json, "ip", "127.0.0.1");
  cJSON_AddNumberToObject (json, "recv_header_sz", 8192);
  cJSON_AddNumberToObject (json, "recv_body_sz", 2048);
  cJSON_AddNumberToObject (json, "resp_header_sz", 16384);
  cJSON_AddNumberToObject (json, "resp_body_sz", 4194304);
  cJSON_AddNumberToObject (json, "timeout_s", 1);
  cJSON_AddNumberToObject (json, "max_clients", 0);
  char *json_text = cJSON_Print (json);
  cJSON_Delete (json);
  return json_text;
}

Test (parse_file, not_enough_clients)
{
  cr_redirect_stderr ();
  setup (zero_clients);
  server_config config = config_make (file_name);
  cr_assert_null (config);
  cr_assert_stderr_eq_str (ERROR_MSG ("max_clients can't be zero or less\n"));
}

Test (_, invalid_file)
{
  cr_redirect_stderr ();
  server_config config = config_make ("_");
  cr_assert_null (config);
  cr_assert_stderr_eq_str (ERROR_MSG ("file _ cannot be opened\n"));
}

Test (_, empty_file)
{
  cr_redirect_stderr ();
  sprintf (file_name, "%d.json", getpid ());
  char expect_msg[256] = { '\0' };
  sprintf (expect_msg, "%sfile %s is empty\n", ERROR_MSG (), file_name);
  FILE *file = fopen (file_name, "w");
  cr_assert_not_null (file);
  fclose (file);
  server_config config = config_make (file_name);
  remove (file_name);
  cr_assert_null (config);
  cr_assert_stderr_eq_str (expect_msg);
}

Test (_, file_not_json)
{
  cr_redirect_stderr ();
  sprintf (file_name, "%d.txt", getpid ());
  char expect_msg[256] = { '\0' };
  sprintf (expect_msg, "%sfile %s is empty\n", ERROR_MSG (), file_name);
  FILE *file = fopen (file_name, "w");
  cr_assert_not_null (file);
  fprintf (file, "{\"\"\"");
  fclose (file);
  server_config config = config_make (file_name);
  remove (file_name);
  cr_assert_null (config);
  cr_assert_stderr_eq_str (ERROR_MSG ("json parsing failed\n"));
}