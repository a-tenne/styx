#define _GNU_SOURCE
#include "header.h"
#include "handle_errs.h"
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void
free_header_list (header node)
{
  if (!node)
    return;
  free_header_list (node->next);
  free (node->key);
  free (node->value);
  free (node);
}

static header
make_header (char *line)
{
  if (!line)
    return NULL;
  regex_t regex;
  int regi = regcomp (&regex,
                      "^[[:alpha:]]+(-[[:alpha:]]+)*\\s*:\\s*\\S+(\\s+\\S+)*$",
                      REG_EXTENDED);
  if (regi != 0)
    {
      regfree (&regex);
      warning ("unable to compile regex.");
      return NULL;
    }
  regi = regexec (&regex, line, 0, NULL, 0);
  regfree (&regex);
  if (regi != 0)
    {
      warning ("invalid header format: %s", line);
      return NULL;
    }
  header head;
  head = malloc (sizeof (*head));
  if (!head)
    {
      warning ("cannot allocate memory for header %s", line);
      return NULL;
    }
  char *saveptr;
  char *key = strtok_r (line, ": ", &saveptr);
  /* if (!key)
    return NULL; */
  char *value = strtok_r (NULL, ": ", &saveptr);
  /* if (!value)
    return NULL; */

  head->key = strdup (key);
  head->value = strdup (value);
  if (!(head->key && head->value))
    {
      free_header_list (head);
      warning ("cannot allocate memory for header data %s", line);
      return NULL;
    }
  head->next = NULL;
  return head;
}

void
free_data (header_data *data)
{
  if (data == NULL)
    return;
  free (data->method);
  free (data->path);
  free (data->version);
  free_header_list (data->header_list.head);
  memset (data, 0, sizeof (*data));
}

static void
meta_data (header_data *data, char *first_line)
{
  char *saveptr;
  char *method = strtok_r (first_line, " ", &saveptr);
  char *path = strtok_r (NULL, " ", &saveptr);
  char *version = strtok_r (NULL, " ", &saveptr);

  if (!(method && path && version))
    warning ("meta data < 3");

  if (strtok_r (NULL, " ", &saveptr))
    warning ("meta data > 3");

  data->method = malloc (strlen (method) + 1);
  if (!data->method)
    {
      warning ("cannot allocate memory for method");
      return;
    }
  data->path = malloc (strlen (path) + 1);
  if (!data->path)
    {
      free (data->method);
      warning ("cannot allocate memory for path");
      return;
    }
  data->version = malloc (strlen (version) + 1);
  if (!data->version)
    {
      free (data->method);
      free (data->path);
      warning ("cannot allocate memory for version");
      return;
    }
  strcpy (data->method, method);
  strcpy (data->path, path);
  strcpy (data->version, version);
}

header_data *
parse (char *header_str)
{
  static header_data data;
  char *saveptr;
  char *token = strtok_r (header_str, "\r\n", &saveptr);
  if (!token)
    {
      warning ("Empty header.");
      return NULL;
    }
  meta_data (&data, token);
  token = strtok_r (NULL, "\r\n", &saveptr);
  header list_item = make_header (token);
  data.header_list.head = list_item;
  while (token && list_item)
    {
      token = strtok_r (NULL, "\r\n", &saveptr);
      list_item->next = make_header (token);
      list_item = list_item->next;
    }
  if ((void *)token != (void *)list_item)
    {
      free_data (&data);
      return NULL;
    }
  return &data;
}