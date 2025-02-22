#include "config.h"
#include "cJSON.h"
#include "handle_errs.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void
free_ressources (server_config *config, cJSON *json)
{
  if (config)
    free (config);
  if (json)
    cJSON_Delete (json);
}

static void
map_vals (server_config *config, cJSON *json)
{
  unsigned char flags = 0;
  char *json_format = "{\n"
                      "\t\"port\": <number>,\n"
                      "\t\"ip\": <string>,\n"
                      "\t\"recv_header_sz\": <number>,\n"
                      "\t\"recv_body_sz\": <number>,\n"
                      "\t\"resp_header_sz\": <number>,\n"
                      "\t\"resp_body_sz\": <number>,\n"
                      "\t\"timeout_s\": <number>,\n"
                      "\t\"max_clients\": <number>\n"
                      "}";
  long numval = 0;
  cJSON *origin = json;
  if (!(json = json->child))
    {
      free_ressources (config, origin);
      exit_error ("json incomplete. Expected format: %s", json_format);
    }
  while (json != NULL)
    {
      switch (json->type)
        {
        case cJSON_String:
          if (strcmp (json->string, "ip"))
            {
              free_ressources (config, origin);
              exit_error ("Invalid attribute in json. Expected format: %s",
                          json_format);
            }
          if (IS_DUP (flags, ADDR))
            warning ("ip address set twice");
          else
            SET_FLAG (flags, ADDR);
          strncpy (config->addr, json->valuestring, 15);
          break;
        case cJSON_Number:
          numval = (long)json->valuedouble;
          if ((double)numval != json->valuedouble)
            {
              free_ressources (config, origin);
              exit_error ("config does not accept floating point numbers");
            }

          if (!strcmp (json->string, "port"))
            {
              if (IS_DUP (flags, PORT))
                warning ("port set twice");
              else
                SET_FLAG (flags, PORT);
              if (numval < 1 || numval > UINT16_MAX)
                {
                  free_ressources (config, origin);
                  exit_error ("invalid port number: %ld", numval);
                }
              config->port = numval;
            }
          else if (!strcmp (json->string, "recv_header_sz"))
            {
              if (IS_DUP (flags, RECV_HEAD))
                warning ("recv_header_sz set twice");
              else
                SET_FLAG (flags, RECV_HEAD);
              config->recv_header_sz = numval;
            }
          else if (!strcmp (json->string, "recv_body_sz"))
            {
              if (IS_DUP (flags, RECV_BODY))
                warning ("recv_body_sz set twice");
              else
                SET_FLAG (flags, RECV_BODY);
              config->recv_body_sz = numval;
            }
          else if (!strcmp (json->string, "resp_header_sz"))
            {
              if (IS_DUP (flags, RESP_HEAD))
                warning ("resp_header_sz set twice");
              else
                SET_FLAG (flags, RESP_HEAD);

              config->resp_header_sz = numval;
            }
          else if (!strcmp (json->string, "resp_body_sz"))
            {
              if (IS_DUP (flags, RESP_BODY))
                warning ("resp_body_sz set twice");
              else
                SET_FLAG (flags, RESP_BODY);
              config->resp_body_sz = numval;
            }
          else if (!strcmp (json->string, "timeout_s"))
            {
              if (IS_DUP (flags, TIMEOUT))
                warning ("timeout_s set twice");
              else
                SET_FLAG (flags, TIMEOUT);
              config->timeout_s = numval;
            }
          else if (!strcmp (json->string, "max_clients"))
            {
              if (numval <= 0)
                {
                  free_ressources (config, origin);
                  exit_error ("max_clients can't be zero or less");
                }
              if (IS_DUP (flags, CLIENTS))
                warning ("max_clients set twice");
              else
                SET_FLAG (flags, CLIENTS);
              if (numval > INT_MAX)
                {
                  free_ressources (config, origin);
                  exit_error ("maximum value for max_clients(%d) exceeded.",
                              INT_MAX);
                }
              config->max_clients = numval;
            }
          else
            {
              free_ressources (config, origin);
              exit_error ("Invalid attribute in json. Expected format: %s",
                          json_format);
            }
          break;
        default:
          free_ressources (config, origin);
          exit_error ("Invalid attribute in json. Expected format: %s",
                      json_format);
        }
      json = json->next;
    }
  if (flags != UCHAR_MAX)
    {
      free_ressources (config, origin);
      exit_error ("json incomplete. Expected format: %s", json_format);
    }
}
server_config *
load_config (const char *file_name)
{
  server_config *config = calloc (1, sizeof (*config));
  if (!config)
    {
      exit_error ("calloc() failed in load_config() for config");
    }
  FILE *file = fopen (file_name, "r");
  if (!file)
    {
      free (config);
      exit_error ("serverconfig.json cannot be opened");
    }
  if (fseek (file, 0, SEEK_END) == -1)
    {
      fclose (file);
      free (config);
      exit_error ("cannot find end of serverconfig.json");
    }
  long buf_size = ftell (file);
  char buf[buf_size + 1];
  memset (buf, 0, buf_size + 1);
  if (buf_size <= 0)
    {
      fclose (file);
      free (config);
      exit_error ("cannot read serverconfig.json");
    }
  rewind (file);
  size_t bytes_read = fread (buf, buf_size, 1, file);
  fclose (file);
  if (bytes_read != 1)
    {
      free (config);
      exit_error ("serverconfig.json cannot be read correctly");
    }
  cJSON *json = cJSON_Parse (buf);
  if (!json)
    {
      free (config);
      exit_error (cJSON_GetErrorPtr ());
    }
  map_vals (config, json);
  cJSON_Delete (json);
  return config;
}