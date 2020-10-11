#include "mediator.h"
#include "applemidi.h"

#define MAX_UDP_DATA_LENGTH 1460

mw_err mediator_recv_event(void)
{
    char buffer[MAX_UDP_DATA_LENGTH];
    s16 length = sizeof(buffer);
    u8 ch;
    mw_err err = mw_recv_sync(&ch, buffer, &length, 0);
    if (err != MW_ERR_NONE) {
        return err;
    }
    switch (ch) {
    case CH_CONTROL_PORT:
        return applemidi_process_control_data(buffer, length);
    case CH_MIDI_PORT:
        return applemidi_process_midi_data(buffer, length);
    }

    return MW_ERR_NONE;
}
