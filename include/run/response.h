#pragma once
#include "header.h"
#include "mem.h"
#include "state.h"

void response (message_buffers *bufs, header_data *request_data,
               connection_state *state);