#include "debug.h"
#include <stdio.h>

void
print_data (header_data *data)
{
  if (!data)
    {
      printf ("NULL\n");
    }
  else
    {
      printf ("header_data: {\n"
              "\tmethod: \"%s\"\n"
              "\tpath: \"%s\"\n"
              "\tversion: \"%s\"\n"
              "\theaders: {\n",
              data->method ? data->method : "NULL",
              data->path ? data->path : "NULL",
              data->version ? data->version : "NULL");

      for (size_t i = 0; i < data->h_vec.length; ++i)
        {
          printf ("\t\t\"%s\": ", data->h_vec.h_array[i].key
                                      ? data->h_vec.h_array[i].key
                                      : "NULL");
          printf ("\"%s\"\n", data->h_vec.h_array[i].value
                                  ? data->h_vec.h_array[i].value
                                  : "NULL");
        }
      printf ("\t}\n}\n");
    }
}