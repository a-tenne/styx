#pragma once
#include "header.h"
#include "mem.h"
#include "state.h"
#include <stdbool.h>

header_data *request (message_buffers *bufs, connection_state *state);