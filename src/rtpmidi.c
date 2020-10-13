#include "rtpmidi.h"
#include "midi.h"

static bool isLongHeader(char* commandSection)
{
    return (u8)commandSection[0] >> 7;
}

static u16 fourBitMidiLength(char* commandSection)
{
    return commandSection[0] & 0x0F;
}

static u16 twelveBitMidiLength(char* commandSection)
{
    return (((u16)commandSection[0] << 12) + (u16)commandSection[1]);
}

static u8 bytesToEmit(u8 status)
{
    if ((status & 0xC0) == 0xC0 || (status & 0xD0) == 0xD0) {
        return 1;
    } else {
        return 2;
    }
}

static void emitMidiEvent(u8 status, char** cursor)
{
    midi_emit(status);
    for (u8 i = 0; i < bytesToEmit(status); i++) {
        midi_emit(**cursor);
        (*cursor)++;
    };
}

mw_err rtpmidi_processRtpMidiPacket(char* buffer, u16 length)
{
    char* commandSection = &buffer[RTP_MIDI_HEADER_LEN];
    bool longHeader = isLongHeader(commandSection);
    u16 midiLength = longHeader ? twelveBitMidiLength(commandSection)
                                : fourBitMidiLength(commandSection);
    char* midiStart = &commandSection[longHeader ? 2 : 1];
    char* midiEnd = &midiStart[midiLength];
    u8 status = 0;
    char* cursor = midiStart;
    while (cursor != midiEnd) {
        if (*cursor & 0x80) { // status bit present
            status = *cursor;
            cursor++;
            continue;
        }

        emitMidiEvent(status, &cursor);
        if (cursor == midiEnd) {
            break;
        }
        // fast forward over high delta time octets
        while (*cursor & 0x80) { cursor++; }
        // skip over final low delta time octet
        cursor++;
    }

    return MW_ERR_NONE;
}
