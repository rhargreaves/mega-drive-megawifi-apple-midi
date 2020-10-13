#pragma once

#include "mediator.h"
#include <types.h>

void __wrap_VDP_drawText(const char* str, u16 x, u16 y);
void __wrap_midi_emit(u8 midiByte);
mw_err __wrap_mediator_recv_event(void);
mw_err __wrap_mediator_send_packet(u8 ch, char* data, u16 len);
void __wrap_SYS_die(char* err);
