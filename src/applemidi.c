#include "applemidi.h"
#include "midi.h"
#include <genesis.h>
#include <stdbool.h>

#include <assert.h>

static mw_err unpack_invitation(
    char* buffer, u16 length, AppleMidiExchangePacket* invite)
{
    if (length < APPLE_MIDI_EXCH_PKT_MIN_LEN) {
        return ERR_APPLE_MIDI_EXCH_PKT_TOO_SMALL;
    }

    u8 index = 0;
    while (index < EXCHANGE_PACKET_LEN) {
        invite->byte[index] = buffer[index];
        index++;
    }
    if (invite->signature != APPLE_MIDI_SIGNATURE) {
        return ERR_INVALID_APPLE_MIDI_SIGNATURE;
    }

    return MW_ERR_NONE;
}

static void pack_invitation_response(u32 initToken, char* buffer, u16* length)
{
    AppleMidiExchangePacket response = { .signature = APPLE_MIDI_SIGNATURE,
        .command = "OK",
        .name = "MegaDrive",
        .initToken = initToken,
        .senderSSRC = MEGADRIVE_SSRC,
        .version = 2 };
    *length = 0;
    while (*length < EXCHANGE_PACKET_LEN) {
        buffer[*length] = response.byte[*length];
        (*length)++;
    }
}

static mw_err send_invite_reply(u8 ch, AppleMidiExchangePacket* invite)
{
    char buffer[UDP_PKT_BUFFER_LEN];
    u16 length;
    pack_invitation_response(invite->initToken, buffer, &length);
    mw_err err = mw_send_sync(ch, buffer, length, 0);
    if (err != MW_ERR_NONE) {
        return err;
    }

    return MW_ERR_NONE;
}

static mw_err unpack_timestamp_sync(
    char* buffer, u16 length, AppleMidiTimeSyncPacket* timeSyncPacket)
{
    if (length != TIMESYNC_PKT_LEN) {
        return ERR_INVALID_TIMESYNC_PKT_LENGTH;
    }
    u8 index = 0;
    while (index < TIMESYNC_PKT_LEN) {
        timeSyncPacket->byte[index] = buffer[index];
        index++;
    }
    if (timeSyncPacket->signature != APPLE_MIDI_SIGNATURE) {
        return ERR_INVALID_APPLE_MIDI_SIGNATURE;
    }
    return MW_ERR_NONE;
}

static void pack_timestamp_sync(
    AppleMidiTimeSyncPacket* timeSyncPacket, char* buffer, u16* length)
{
    *length = 0;
    while (*length < TIMESYNC_PKT_LEN) {
        buffer[*length] = timeSyncPacket->byte[*length];
        (*length)++;
    }
}

static mw_err send_timesync(AppleMidiTimeSyncPacket* timeSyncPacket)
{
    char buffer[TIMESYNC_PKT_LEN];
    u16 length;
    pack_timestamp_sync(timeSyncPacket, buffer, &length);
    mw_err err = mw_send_sync(CH_MIDI_PORT, buffer, length, 0);
    if (err != MW_ERR_NONE) {
        return err;
    }
    return MW_ERR_NONE;
}

static mw_err process_invitation(u8 ch, char* buffer, u16 length)
{
    AppleMidiExchangePacket packet;
    mw_err err = unpack_invitation(buffer, length, &packet);
    char text[100];
    sprintf(text, "Invited on ch %d:", ch);
    VDP_drawText(text, 1, 10 + ch);

    err = send_invite_reply(ch, &packet);
    if (err != MW_ERR_NONE) {
        return err;
    }
    sprintf(text, "Responded");
    VDP_drawText(text, 20, 10 + ch);

    return MW_ERR_NONE;
}

static u16 timestampSyncCount = 0;

static mw_err process_timestamp_sync(char* buffer, u16 length)
{
    AppleMidiTimeSyncPacket packet;
    mw_err err = unpack_timestamp_sync(buffer, length, &packet);
    if (err != MW_ERR_NONE) {
        return err;
    }
    if (packet.count == 0) {
        packet.count = 1;
        packet.timestamp2Hi = 0;
        packet.timestamp2Lo = 0;
        packet.senderSSRC = MEGADRIVE_SSRC;
        err = send_timesync(&packet);
        if (err != MW_ERR_NONE) {
            return err;
        }

        char text[32];
        sprintf(text, "Timestamp Syncs: %d", timestampSyncCount++);
        VDP_drawText(text, 1, 17);
    }

    return MW_ERR_NONE;
}

static bool has_apple_midi_sig(char* buffer, u16 length)
{
    if (length < 2) {
        return false;
    }
    return *((u16*)buffer) == APPLE_MIDI_SIGNATURE;
}

static bool is_command_invitation(char* command)
{
    return command[0] == 'I' && command[1] == 'N';
}

static bool is_command_timestamp_sync(char* command)
{
    return command[0] == 'C' && command[1] == 'K';
}

mw_err applemidi_process_control_data(char* buffer, u16 length)
{
    if (!has_apple_midi_sig(buffer, length)) {
        return ERR_INVALID_APPLE_MIDI_SIGNATURE;
    }

    char* command = &buffer[2];
    if (is_command_invitation(command)) {
        return process_invitation(CH_CONTROL_PORT, buffer, length);
    }

    return MW_ERR_NONE;
}

static bool isLongHeader(char* commandSection)
{
    return (u8)commandSection[0] >> 7;
}

static u16 FourBitMidiLength(char* commandSection)
{
    return commandSection[0] & 0x0F;
}

static u16 TwelveBitMidiLength(char* commandSection)
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

static void emitMidiEvent(u8 currentStatus, char** cursor)
{
    midi_emit(currentStatus);
    for (u8 i = 0; i < bytesToEmit(currentStatus); i++) {
        midi_emit(**cursor);
        (*cursor)++;
    };
}

mw_err process_rtp_midi(char* buffer, u16 length)
{
    char* commandSection = &buffer[RTP_MIDI_HEADER_LEN];
    bool longHeader = isLongHeader(commandSection);
    u16 midiLength = longHeader ? TwelveBitMidiLength(commandSection)
                                : FourBitMidiLength(commandSection);
    char* midiStart = &commandSection[longHeader ? 2 : 1];
    char* midiEnd = &midiStart[midiLength];
    u8 currentStatus = 0;
    char* cursor = midiStart;
    while (cursor != midiEnd) {

        if (*cursor & 0x80) { // status bit
            currentStatus = *cursor;
            cursor++;
            continue;
        }

        emitMidiEvent(currentStatus, &cursor);
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

mw_err applemidi_process_midi_data(char* buffer, u16 length)
{
    if (has_apple_midi_sig(buffer, length)) {
        char* command = &buffer[2];
        if (is_command_invitation(command)) {
            return process_invitation(CH_MIDI_PORT, buffer, length);
        } else if (is_command_timestamp_sync(command)) {
            return process_timestamp_sync(buffer, length);
        } else {
            char text[100];
            sprintf(text, "Unknown event %s", command);
            VDP_drawText(text, 1, 14);
        }
    } else {
        return process_rtp_midi(buffer, length);
    }

    return MW_ERR_NONE;
}
