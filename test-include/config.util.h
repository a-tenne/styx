#ifndef _CONFIG_UTIL_H
#define _CONFIG_UTIL_H
#include "config.h"

extern char file_name[100];
extern server_config config;
extern const char *argv[];

extern void setup_config (int port);
extern void teardown_config (void);

#endif