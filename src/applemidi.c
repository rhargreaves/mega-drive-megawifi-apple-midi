#include "applemidi.h"
#include <genesis.h>

static mw_err unpack_invitation(
    char* buffer, u16 length, AppleMidiExchangePacket* invite)
{
    if (length < APPLE_MIDI_EXCH_PKT_MIN_LEN) {
        return ERR_APPLE_MIDI_EXCH_PKT_TOO_SMALL;
    }

    u8 index = 0;
    while (index < UDP_PKT_LEN) { invite->byte[index] = buffer[index++]; }
    if (invite->signature != APPLE_MIDI_SIGNATURE) {
        return ERR_INVALID_APPLE_MIDI_SIGNATURE;
    }

    return MW_ERR_NONE;
}

mw_err receive_invitation(u8 ch, AppleMidiExchangePacket* invite)
{
    char buffer[UDP_PKT_BUFFER_LEN];
    s16 buf_length = sizeof(buffer);
    u8 actualCh;
    mw_err err = mw_recv_sync(&actualCh, buffer, &buf_length, 0);
    if (err != MW_ERR_NONE) {
        return err;
    }
    if (actualCh != ch) {
        return ERR_UNEXPECTED_CHANNEL;
    }

    return unpack_invitation(buffer, buf_length, invite);
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
    while (*length < UDP_PKT_LEN) {
        buffer[*length] = response.byte[(*length)++];
    }
}

mw_err send_invite_reply(u8 ch, AppleMidiExchangePacket* invite)
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
        timeSyncPacket->byte[index] = buffer[index++];
    }
    if (timeSyncPacket->signature != APPLE_MIDI_SIGNATURE) {
        return ERR_INVALID_APPLE_MIDI_SIGNATURE;
    }
    return MW_ERR_NONE;
}

mw_err recv_timesync(AppleMidiTimeSyncPacket* timeSyncPacket)
{
    char buffer[TIMESYNC_PKT_LEN];
    u16 buf_length = sizeof(buffer);
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

mw_err send_timesync(AppleMidiTimeSyncPacket* timeSyncPacket)
{
    char buffer[TIMESYNC_PKT_LEN];
    s16 buf_length = sizeof(buffer);
    u8 index = 0;
    while (index < TIMESYNC_PKT_LEN) {
        buffer[index] = timeSyncPacket->byte[index++];
    }
    mw_err err = mw_send_sync(CH_MIDI_PORT, buffer, index, 0);
    if (err != MW_ERR_NONE) {
        return err;
    }
    return MW_ERR_NONE;
}

mw_err handshake(u8 ch)
{
    mw_err err = MW_ERR_NONE;

    AppleMidiExchangePacket packet;
    err = receive_invitation(ch, &packet);
    if (err != MW_ERR_NONE) {
        return err;
    }
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

mw_err timesync(void)
{
    AppleMidiTimeSyncPacket packet;
    mw_err err = recv_timesync(&packet);
    if (err != MW_ERR_NONE) {
        return err;
    }
    packet.count = 1;
    packet.timestamp2Hi = 0;
    packet.timestamp2Lo = 0;
    packet.senderSSRC = MEGADRIVE_SSRC;
    err = send_timesync(&packet);
    if (err != MW_ERR_NONE) {
        return err;
    }
    err = recv_timesync(&packet);
    if (err != MW_ERR_NONE) {
        return err;
    }
    return err;
}
