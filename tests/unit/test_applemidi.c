#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include "unused.h"
#include <cmocka.h>
#include <stdio.h>

#include "applemidi.h"

static int test_applemidi_setup(UNUSED void** state)
{
    return 0;
}

static void test_applemidi_parses_rtpmidi_packet(UNUSED void** state)
{
    char rtp_packet[1024]
        = { /* V P X CC M PT */ 0x80, 0x61, /* sequence number */ 0x8c, 0x24,
              /* timestamp */ 0x00, 0x58, 0xbb, 0x40, /* SSRC */ 0xac, 0x67,
              0xe1, 0x08, /* MIDI command section */ 0x06, 0x90, 0x48, 0x6f,
              0x00, 0x52, 0x73 };

    size_t len = sizeof(rtp_packet);

    mw_err err = applemidi_process_midi_data(rtp_packet, len);

    assert_int_equal(err, MW_ERR_NONE);
}
