#include "config.h"
#include "cJSON.h"
#include <criterion/criterion.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

char file_name[100] = { '\0' };
server_config config = NULL;
const char *argv[] = { "./Tests", file_name, NULL };

void
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

void
teardown_config (void)
{
  config_destroy (&config);
  remove (file_name);
}