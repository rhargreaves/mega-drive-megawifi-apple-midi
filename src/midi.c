#include "midi.h"
#include <genesis.h>

static u16 y = 10;

void midi_emit(u8 midiByte)
{
    char text[32];
    sprintf(text, "MIDI emit: 0x%x", midiByte);
    VDP_drawText(text, 1, y++);
}
