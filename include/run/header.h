#pragma once
#include <stddef.h>
typedef struct
{
  char *key;
  char *value;
} header;

typedef struct
{
  header *h_array;
  size_t length;
  size_t capacity;
} header_vec;

typedef struct
{
  char *method;
  char *path;
  char *version;
  header_vec h_vec;
} header_data;

header_data *parse (char *header_str);
void free_data (header_data *data);
char *vec_lookup (header_vec *vec, const char *key);