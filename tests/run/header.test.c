#include "header.h"
#include "errlog.h"
#include <criterion/criterion.h>
#include <criterion/redirect.h>

Test (_, header_str_not_null)
{
  cr_redirect_stderr ();
  header_data *data = parse (NULL);
  cr_assert_null (data);
  cr_assert_stderr_eq_str (ERROR_MSG ("header_str cannot be NULL\n"));
}

Test (_, empty_header)
{
  cr_redirect_stdout ();
  char input[] = "";

  header_data *data = parse (input);
  cr_assert_null (data);

  cr_assert_stdout_eq_str (WARN_MSG ("empty header string\n"));
}

Test (_, invalid_header_titles)
{
  cr_redirect_stdout ();
  char input1[] = "test\r";
  char input2[] = "test\n";
  char input3[] = " /path HTTP/1.1\r\n";
  char input4[] = "HEAD / HTTP/\r\n";
  char input5[] = "get / HTTP/1.1\r\n";
  header_data *data = parse (input1);
  cr_assert_null (data);
  data = parse (input2);
  cr_assert_null (data);
  data = parse (input3);
  cr_assert_null (data);
  data = parse (input4);
  cr_assert_null (data);
  data = parse (input5);
  cr_assert_null (data);
  cr_assert_stdout_eq_str (
      WARN_MSG ("invalid header format: test\n")
          WARN_MSG ("invalid header format: test\n")
              WARN_MSG ("invalid header format:  /path HTTP/1.1\n")
                  WARN_MSG ("invalid header format: HEAD / HTTP/\n")
                      WARN_MSG ("invalid header format: get / HTTP/1.1\n"));
}

Test (_, parses_sigle_title)
{
  char input[] = "GET / HTTP/1.1\r\n";
  header_data *data = parse (input);
  cr_assert_not_null (data);
  free_data (data);
}

Test (_, invalid_header_format)
{
  cr_redirect_stdout ();
  char input1[] = "GET /path/to/file HTTP/1.1\r\n"
                  "Connection: \r\n";
  char input2[] = "GET /path/to/file HTTP/1.1\r\n"
                  "Connection ";
  header_data *data = parse (input1);
  cr_assert_null (data);
  data = parse (input2);
  cr_assert_null (data);
  cr_assert_stdout_eq_str (
      WARN_MSG ("invalid header format: Connection: \n")
          WARN_MSG ("invalid header format: Connection \n"));
}

Test (_, parses_regular_header)
{
  char input[] = "GET /path/to/file HTTP/1.1\r\n"
                 "Connection: keep-alive\r\n"
                 "User-Agent: Test\r\n";
  header_data *data = parse (input);
  cr_assert_not_null (data);
  cr_assert_str_eq (data->method, "GET");
  cr_assert_str_eq (data->path, "/path/to/file");
  cr_assert_str_eq (data->version, "HTTP/1.1");
  cr_assert_str_eq (vec_lookup (&data->h_vec, "Connection"), "keep-alive");
  cr_assert_str_eq (vec_lookup (&data->h_vec, "User-Agent"), "Test");
  free_data (data);
}