#define _GNU_SOURCE
#include "header.h"
#include "handle_errs.h"
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void
vec_free (header_vec *vec)
{
  for (size_t i = 0; i < vec->length; ++i)
    {
      free (vec->h_array[i].key);
      free (vec->h_array[i].value);
    }
  free (vec->h_array);
}

static bool
vec_append (header_vec *vec, const char *key, const char *value)
{
  void *temp;
  if (vec->h_array == NULL)
    {
      vec->capacity = 8;
      vec->h_array = malloc (sizeof (*vec->h_array) * 10);
      if (vec->h_array == NULL)
        {
          warning ("cannot allocate header array");
          return false;
        }
    }
  else if (vec->length == vec->capacity)
    {
      temp
          = realloc (vec->h_array, vec->capacity * 2 * sizeof (*vec->h_array));
      if (temp == NULL)
        {
          warning ("cannot reallocate header array");
          return false;
        }
      vec->capacity *= 2;
      vec->h_array = temp;
    }
  char *key_copy = strdup (key);
  char *value_copy = strdup (value);
  if (key_copy == NULL)
    {
      warning ("cannot allocate memory for key: %s", key);
      return false;
    }
  if (value_copy == NULL)
    {
      free (key_copy);
      warning ("cannot allocate memory for value: %s", value);
      return false;
    }
  vec->h_array[vec->length].key = key_copy;
  vec->h_array[vec->length].value = value_copy;
  ++vec->length;
  return true;
}

static bool
vec_append_line (header_vec *vec, char *line)
{
  if (!line)
    return false;
  regex_t regex;
  int regi = regcomp (&regex,
                      "^[[:alpha:]]+(-[[:alpha:]]+)*\\s*:\\s*\\S+(\\s+\\S+)*$",
                      REG_EXTENDED);
  if (regi != 0)
    {
      regfree (&regex);
      warning ("unable to compile regex.");
      return false;
    }
  regi = regexec (&regex, line, 0, NULL, 0);
  regfree (&regex);
  if (regi != 0)
    {
      warning ("invalid header format: %s", line);
      return false;
    }
  char *saveptr;
  char *key = strtok_r (line, ": ", &saveptr);
  char *value = strtok_r (NULL, ": ", &saveptr);
  return vec_append (vec, key, value);
}

void
free_data (header_data *data)
{
  if (data == NULL)
    return;
  free (data->method);
  free (data->path);
  free (data->version);
  vec_free (&data->h_vec);
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
  memset (&data, 0, sizeof (data));
  char *saveptr;
  char *token = strtok_r (header_str, "\r\n", &saveptr);
  if (!token)
    {
      warning ("Empty header.");
      return NULL;
    }
  meta_data (&data, token);

  bool result = true;
  token = strtok_r (NULL, "\r\n", &saveptr);
  while (token && result)
    {
      result = vec_append_line (&data.h_vec, token);
      token = strtok_r (NULL, "\r\n", &saveptr);
    }
  if (token == NULL && !result)
    {
      free_data (&data);
      return NULL;
    }
  return &data;
}

char *
vec_lookup (header_vec *vec, const char *key)
{
  for (size_t i = 0; i < vec->length; ++i)
    {
      if (!strcasecmp (key, vec->h_array[i].key))
        return vec->h_array[i].value;
    }
  return NULL;
}