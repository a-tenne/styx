#include "config.h"
#include "cJSON.h"
#include "errlog.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PORT (1 << 0)
#define ADDR (1 << 1)
#define RECV_HEAD (1 << 2)
#define RECV_BODY (1 << 3)
#define RESP_HEAD (1 << 4)
#define RESP_BODY (1 << 5)
#define TIMEOUT (1 << 6)
#define CLIENTS (1 << 7)
#define IS_DUP(flags, mask) (flags & mask)
#define SET_FLAG(flags, mask) (flags |= mask)

static void
free_ressources (server_config *config, cJSON *json)
{
  if (config)
    config_destroy (config);
  if (json)
    cJSON_Delete (json);
}

static void
config_map (server_config config, cJSON *json)
{
  uint8_t flags = 0;
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
  if ((json = json->child) == NULL)
    {
      free_ressources (&config, origin);
      EXIT_ERROR (, "json incomplete. Expected format: %s", json_format);
    }
  while (json != NULL)
    {
      switch (json->type)
        {
        case cJSON_String:
          if (strcmp (json->string, "ip"))
            {
              free_ressources (&config, origin);
              EXIT_ERROR (, "Invalid attribute in json. Expected format: %s",
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
              free_ressources (&config, origin);
              EXIT_ERROR (, "config does not accept floating point numbers");
            }

          if (!strcmp (json->string, "port"))
            {
              if (IS_DUP (flags, PORT))
                warning ("port set twice");
              else
                SET_FLAG (flags, PORT);
              if (numval < 1 || numval > UINT16_MAX)
                {
                  free_ressources (&config, origin);
                  EXIT_ERROR (, "invalid port number: %ld", numval);
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
                  free_ressources (&config, origin);
                  EXIT_ERROR (, "max_clients can't be zero or less");
                }
              if (IS_DUP (flags, CLIENTS))
                warning ("max_clients set twice");
              else
                SET_FLAG (flags, CLIENTS);
              if (numval > INT_MAX)
                {
                  free_ressources (&config, origin);
                  EXIT_ERROR (, "maximum value for max_clients(%d) exceeded.",
                              INT_MAX);
                }
              config->max_clients = numval;
            }
          else
            {
              free_ressources (&config, origin);
              EXIT_ERROR (, "Invalid attribute in json. Expected format: %s",
                          json_format);
            }
          break;
        default:
          free_ressources (&config, origin);
          EXIT_ERROR (, "Invalid attribute in json. Expected format: %s",
                      json_format);
        }
      json = json->next;
    }
  if (flags != UINT8_MAX)
    {
      free_ressources (&config, origin);
      EXIT_ERROR (, "json incomplete. Expected format: %s", json_format);
    }
}
server_config
config_make (const char *file_name)
{
  NULL_CHECK (file_name, NULL);
  server_config config = calloc (1, sizeof (*config));
  if (config == NULL)
    {
      EXIT_ERROR (NULL, "cannot allocate memory for config");
    }
  FILE *file = fopen (file_name, "r");
  if (file == NULL)
    {
      config_destroy (&config);
      EXIT_ERROR (NULL, "file %s cannot be opened", file_name);
    }
  if (fseek (file, 0, SEEK_END) == -1)
    {
      fclose (file);
      config_destroy (&config);
      EXIT_ERROR (NULL, "cannot find end of serverconfig.json");
    }
  long buf_size = ftell (file);
  char buf[buf_size + 1];
  memset (buf, 0, buf_size + 1);
  if (buf_size <= 0)
    {
      fclose (file);
      config_destroy (&config);
      EXIT_ERROR (NULL, "cannot read serverconfig.json");
    }
  rewind (file);
  size_t bytes_read = fread (buf, buf_size, 1, file);
  fclose (file);
  if (bytes_read != 1)
    {
      config_destroy (&config);
      EXIT_ERROR (NULL, "serverconfig.json cannot be read correctly");
    }
  cJSON *json = cJSON_Parse (buf);
  if (json == NULL)
    {
      config_destroy (&config);
      EXIT_ERROR (NULL, cJSON_GetErrorPtr ());
    }
  config_map (config, json);
  cJSON_Delete (json);
  return config;
}

void
config_destroy (server_config *config)
{
  free (*config);
  *config = NULL;
}