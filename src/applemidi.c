#include "applemidi.h"
#include <genesis.h>
#include <stdbool.h>

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

static mw_err recv_timesync(AppleMidiTimeSyncPacket* timeSyncPacket)
{
    char buffer[TIMESYNC_PKT_LEN];
    s16 buf_length = sizeof(buffer);
    u8 actualCh;
    mw_err err = mw_recv_sync(&actualCh, buffer, &buf_length, 0);
    if (err != MW_ERR_NONE) {
        return err;
    }
    if (actualCh != CH_MIDI_PORT) {
        return ERR_UNEXPECTED_CHANNEL;
    }

    return unpack_timestamp_sync(buffer, buf_length, timeSyncPacket);
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

mw_err process_rtp_midi(char* buffer, u16 length)
{
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
