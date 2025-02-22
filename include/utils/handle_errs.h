/**
 * @file utils.h
 * @brief Small utility functions for the WebServer project
 *
 * Contains small helper functions that are used across the project.
 */

#pragma once
#include <stdarg.h>

/**
 * @brief Prints out a formatted error message to stdout with a newline and
 * exits with error code 1.
 * @param error_msg A pointer to the unformatted message to be printed.
 * @param ... The variable arguments that help format the message.
 */
void exit_error (const char *error_msg, ...);

#ifndef TEST
/**
 * @brief Wrapper macro for returning from functions instead of crashing the
 * program, when testing is enabled.
 */
#define EXIT_ERROR(returnval, msg, ...)                                       \
  (exit_error (msg __VA_OPT__ (, ) __VA_ARGS__))
#else
/**
 * @brief Wrapper macro for returning from functions instead of crashing the
 * program, when testing is enabled.
 */
#define EXIT_ERROR(returnval, msg, ...)                                       \
  do                                                                          \
    {                                                                         \
      exit_error (msg __VA_OPT__ (, ) __VA_ARGS__);                           \
      return returnval;                                                       \
    }                                                                         \
  while (0)
#endif

/**
 * @brief Emits a formatted warning message to stdout with a newline.
 * @param warning_msg A pointer to the unformatted message to be printed.
 * @param ... The variable arguments that help format the message.
 */
void warning (const char *warning_msg, ...);
