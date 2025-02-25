/**
 * @file utils.h
 * @brief Small utility functions for the WebServer project
 *
 * Contains small helper functions that are used across the project.
 */

#pragma once
#include <stdarg.h>

/**
 * @brief Appends the input to a template error string.
 * @param msg The constant string to be appended.
 */
#define ERROR_MSG(msg) "\033[1;31mERROR:\033[0m " msg
/**
 * @brief Appends the input to a template warning string.
 * @param msg The constant string to be appended.
 */
#define WARN_MSG(msg) "\033[1;35mWARNING:\033[0m " msg

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
 * @param returnval The value to be returned when testing.
 * @param msg The format error message.
 */
#define EXIT_ERROR(returnval, msg, ...)                                       \
  exit_error (msg __VA_OPT__ (, ) __VA_ARGS__)
#else
/**
 * @brief Wrapper macro for returning from functions instead of crashing the
 * program, when testing is enabled.
 * @param returnval The value to be returned when testing.
 * @param msg The format error message.
 */
#define EXIT_ERROR(returnval, msg, ...)                                       \
  do                                                                          \
    {                                                                         \
      exit_error (msg __VA_OPT__ (, ) __VA_ARGS__);                           \
      return returnval;                                                       \
    }                                                                         \
  while (0)
#endif

#if defined(DEBUG) || defined(TEST)
/**
 * @brief When in debug or testing mode, checks the input pointer for NULL.
 * If it's NULL, print a formatted error message with the pointer's name
 * and crash the program or, when testing, return returnval.
 * @param ptr The pointer to check.
 * @param returnval The value to be returned when testing.
 * @note Does nothing in release mode.
 */
#define NULL_CHECK(ptr, returnval)                                            \
  do                                                                          \
    {                                                                         \
      if (ptr == NULL)                                                        \
        EXIT_ERROR (returnval, "%s cannot be NULL", #ptr);                    \
    }                                                                         \
  while (0)

#else
/**

 * @brief When in debug or testing mode, checks the input pointer for NULL.
 * If it's NULL, print a formatted error message with the pointer's name
 * and crash the program or, when testing, return returnval.
 * @param ptr The pointer to check.
 * @param returnval The value to be returned when testing.
 * @note Does nothing in release mode
 */
#define NULL_CHECK(ptr, returnval)
#endif

/**
 * @brief Emits a formatted warning message to stdout with a newline.
 * @param warning_msg A pointer to the unformatted message to be printed.
 * @param ... The variable arguments that help format the message.
 */
void warning (const char *warning_msg, ...);
