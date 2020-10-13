#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include "unused.h"
#include <cmocka.h>
#include <stdio.h>

#include "applemidi.h"
#include "asserts.h"

static int test_applemidi_setup(UNUSED void** state)
{
    return 0;
}

static void test_applemidi_parses_rtpmidi_packet_with_single_midi_event(
    UNUSED void** state)
{
    char rtp_packet[1024]
        = { /* V P X CC M PT */ 0x80, 0x61, /* sequence number */ 0x8c, 0x24,
              /* timestamp */ 0x00, 0x58, 0xbb, 0x40, /* SSRC */ 0xac, 0x67,
              0xe1, 0x08, /* MIDI command section */ 0x03, 0x90, 0x48, 0x6f };

    size_t len = sizeof(rtp_packet);

    expect_midi_emit(0x90);
    expect_midi_emit(0x48);
    expect_midi_emit(0x6f);

    mw_err err = applemidi_processSessionMidiPacket(rtp_packet, len);
    assert_int_equal(err, MW_ERR_NONE);
}

static void test_applemidi_parses_rtpmidi_packet_with_single_2_byte_midi_event(
    UNUSED void** state)
{
    char rtp_packet[1024]
        = { /* V P X CC M PT */ 0x80, 0x61, /* sequence number */ 0x8c, 0x24,
              /* timestamp */ 0x00, 0x58, 0xbb, 0x40, /* SSRC */ 0xac, 0x67,
              0xe1, 0x08, /* MIDI command section */ 0x02, 0xC0, 0x01 };

    size_t len = sizeof(rtp_packet);

    expect_midi_emit(0xC0);
    expect_midi_emit(0x01);

    mw_err err = applemidi_processSessionMidiPacket(rtp_packet, len);
    assert_int_equal(err, MW_ERR_NONE);
}

static void
test_applemidi_parses_rtpmidi_packet_with_single_midi_event_long_header(
    UNUSED void** state)
{
    char rtp_packet[1024] = { /* V P X CC M PT */ 0x80, 0x61,
        /* sequence number */ 0x8c, 0x24,
        /* timestamp */ 0x00, 0x58, 0xbb, 0x40, /* SSRC */ 0xac, 0x67, 0xe1,
        0x08, /* MIDI command section */ 0xC0, 0x03, 0x90, 0x48, 0x6f };

    size_t len = sizeof(rtp_packet);

    expect_midi_emit(0x90);
    expect_midi_emit(0x48);
    expect_midi_emit(0x6f);

    mw_err err = applemidi_processSessionMidiPacket(rtp_packet, len);
    assert_int_equal(err, MW_ERR_NONE);
}

static void test_applemidi_parses_rtpmidi_packet_with_two_midi_events(
    UNUSED void** state)
{
    char rtp_packet[1024]
        = { /* V P X CC M PT */ 0x80, 0x61, /* sequence number */ 0x8c, 0x24,
              /* timestamp */ 0x00, 0x58, 0xbb, 0x40, /* SSRC */ 0xac, 0x67,
              0xe1, 0x08, /* MIDI command section */ 0x06, 0x90, 0x48, 0x6f,
              0x00, 0x51, 0x6f };

    size_t len = sizeof(rtp_packet);

    expect_midi_emit(0x90);
    expect_midi_emit(0x48);
    expect_midi_emit(0x6f);
    expect_midi_emit(0x90);
    expect_midi_emit(0x51);
    expect_midi_emit(0x6f);

    mw_err err = applemidi_processSessionMidiPacket(rtp_packet, len);
    assert_int_equal(err, MW_ERR_NONE);
}

static void test_applemidi_parses_rtpmidi_packet_with_multiple_midi_events(
    UNUSED void** state)
{
    char rtp_packet[1024]
        = { /* V P X CC M PT */ 0x80, 0x61, /* sequence number */ 0x8c, 0x24,
              /* timestamp */ 0x00, 0x58, 0xbb, 0x40, /* SSRC */ 0xac, 0x67,
              0xe1, 0x08, /* MIDI command section */ 0x09, 0x90, 0x48, 0x6f,
              0x00, 0x51, 0x6f, 0x00, 0x48, 0x6f };

    size_t len = sizeof(rtp_packet);

    expect_midi_emit(0x90);
    expect_midi_emit(0x48);
    expect_midi_emit(0x6f);
    expect_midi_emit(0x90);
    expect_midi_emit(0x51);
    expect_midi_emit(0x6f);
    expect_midi_emit(0x90);
    expect_midi_emit(0x48);
    expect_midi_emit(0x6f);

    mw_err err = applemidi_processSessionMidiPacket(rtp_packet, len);
    assert_int_equal(err, MW_ERR_NONE);
}

static void
test_applemidi_parses_rtpmidi_packet_with_multiple_different_midi_events(
    UNUSED void** state)
{
    char rtp_packet[1024]
        = { /* V P X CC M PT */ 0x80, 0x61, /* sequence number */ 0x8c, 0x24,
              /* timestamp */ 0x00, 0x58, 0xbb, 0x40, /* SSRC */ 0xac, 0x67,
              0xe1, 0x08, /* MIDI command section */ 0x0A, 0x90, 0x48, 0x6f,
              0x00, 0x51, 0x6f, 0x00, 0x80, 0x48, 0x6f };

    size_t len = sizeof(rtp_packet);

    expect_midi_emit(0x90);
    expect_midi_emit(0x48);
    expect_midi_emit(0x6f);
    expect_midi_emit(0x90);
    expect_midi_emit(0x51);
    expect_midi_emit(0x6f);
    expect_midi_emit(0x80);
    expect_midi_emit(0x48);
    expect_midi_emit(0x6f);

    mw_err err = applemidi_processSessionMidiPacket(rtp_packet, len);
    assert_int_equal(err, MW_ERR_NONE);
}
