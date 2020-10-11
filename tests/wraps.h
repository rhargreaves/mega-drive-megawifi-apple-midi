#pragma once

#include <types.h>

void __wrap_VDP_drawText(const char* str, u16 x, u16 y);
void __wrap_midi_emit(u8 midiByte);
